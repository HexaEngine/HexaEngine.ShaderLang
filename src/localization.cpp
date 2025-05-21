#include "pch/localization.hpp"
#include "config.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <zstd.h>

namespace HXSL
{
	const std::string MAGIC_STRING = "TRANSL";
	const uint32_t CURRENT_VERSION = 1;

	std::unique_ptr<std::unordered_map<uint64_t, std::string>> current_locale_map = std::make_unique<std::unordered_map<uint64_t, std::string>>();

	struct ZSTDStream
	{
		std::istream& in;
		ZSTD_DStream* dstream = nullptr;
		std::vector<char> inputBuffer;
		std::vector<char> outputBuffer;
		size_t bufferSize;
		ZSTD_inBuffer inBuffer{};
		ZSTD_outBuffer outBuffer{};

		ZSTDStream(std::istream& in, size_t bufferSize = 1024 * 1024) : in(in), bufferSize(bufferSize), inputBuffer(bufferSize), outputBuffer(bufferSize)
		{
			dstream = ZSTD_createDStream();
			if (!dstream) throw std::runtime_error("Failed to create ZSTD_DStream");
			size_t initResult = ZSTD_initDStream(dstream);
			if (ZSTD_isError(initResult)) throw std::runtime_error(ZSTD_getErrorName(initResult));
			inBuffer = { inputBuffer.data(), 0, 0 };
			outBuffer = { outputBuffer.data(), 0, 0 };
		}

		~ZSTDStream()
		{
			if (dstream) ZSTD_freeDStream(dstream);
		}

		bool DecompressNextChunk()
		{
			if (inBuffer.pos == inBuffer.size)
			{
				in.read(inputBuffer.data(), bufferSize);
				size_t bytesRead = in.gcount();
				if (bytesRead == 0) return false;
				inBuffer = { inputBuffer.data(), bytesRead, 0 };
			}

			outBuffer = { outputBuffer.data(), outputBuffer.size(), 0 };
			size_t result = ZSTD_decompressStream(dstream, &outBuffer, &inBuffer);
			if (ZSTD_isError(result))
			{
				std::cerr << "ZSTD decompression error: " << ZSTD_getErrorName(result) << std::endl;
				return false;
			}
			outBuffer.size = outBuffer.pos;
			outBuffer.pos = 0;
			return outBuffer.size > 0;
		}

		bool Read(void* dst, size_t size)
		{
			size_t copied = 0;
			while (copied < size)
			{
				if (outBuffer.pos == outBuffer.size)
				{
					if (!DecompressNextChunk()) return false;
				}

				size_t available = outBuffer.size - outBuffer.pos;
				size_t toCopy = std::min(size - copied, available);
				std::memcpy((char*)dst + copied, outputBuffer.data() + outBuffer.pos, toCopy);
				outBuffer.pos += toCopy;
				copied += toCopy;
			}
			return true;
		}
	};

	static bool LoadTranslations(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::binary);
		if (!file)
		{
			std::cerr << "Failed to open file: " << filename << std::endl;
			return false;
		}

		char magic[6];
		file.read(magic, MAGIC_STRING.size());

		if (std::string(magic, MAGIC_STRING.size()) != MAGIC_STRING)
		{
			std::cerr << "Invalid magic string!" << std::endl;
			return false;
		}

		uint32_t version = 0;

		file.read(reinterpret_cast<char*>(&version), sizeof(version));

		if (version != CURRENT_VERSION)
		{
			std::cerr << "Version mismatch!" << std::endl;
			return false;
		}

		uint64_t entryCount = 0;
		file.read(reinterpret_cast<char*>(&entryCount), sizeof(entryCount));

		auto& map = *current_locale_map.get();

		map.clear();
		map.reserve(entryCount);

		ZSTDStream stream = ZSTDStream(file);
		for (size_t i = 0; i < entryCount; i++)
		{
			uint64_t key;
			uint32_t strLength;
			if (!stream.Read(&key, sizeof(uint64_t)) ||
				!stream.Read(&strLength, sizeof(uint32_t)))
			{
				std::cerr << "Stream read error for entry metadata" << std::endl;
				return false;
			}

			std::string translation;
			translation.resize(strLength);
			if (!stream.Read(translation.data(), strLength))
			{
				std::cerr << "Stream read error for string data" << std::endl;
				return false;
			}

			map[key] = translation;
		}

		return true;
	}

	void SetLocale(const std::string& languageCode)
	{
		std::filesystem::path base = HXSL_LOCALE_PATH;
		std::filesystem::path path = base / (languageCode + ".transl");
		LoadTranslations(path.string());
	}

	std::string GetMessageForCode(uint64_t code)
	{
		if (current_locale_map != nullptr)
		{
			code &= ~(0x3ull << 62ull);
			auto msg_it = current_locale_map->find(code);
			if (msg_it != current_locale_map->end())
			{
				return msg_it->second;
			}
		}

		return "Unknown localization code";
	}

	const static char lut[11] = { 0, 4, 7, 10, 14, 17, 20, 24, 27, 30, 31 };
	constexpr size_t one = 1;
	constexpr size_t ten = 10;

	static uint32_t EncodeWithLeading(const std::string& input, size_t start, size_t length)
	{
		uint32_t number = 0;

		for (size_t i = start; i < length; i++)
		{
			auto c = input[i];
			if (std::isdigit(c))
			{
				number = number * ten + static_cast<size_t>(c - '0');
			}
			else
			{
				throw std::runtime_error("Unexpected character in diagnostic code id.");
			}
		}

		size_t range = length - start;
		size_t startingBit = (range < 11) ? lut[range] : lut[10];

		return number | (one << startingBit);
	}

	static uint32_t HighestSetBitPosition(uint32_t x)
	{
#if defined(_MSC_VER)
		unsigned long index;
		return _BitScanReverse(&index, x) ? index : 0;
#else
		return 31 - __builtin_clz(x);
#endif
	}

	static const char lutBack[32] =
	{
		0,  // special bit does not contribute to digits
		1,  // 2^0
		1,  // 2^1
		1,  // 2^2
		2,  // 2^3
		2,  // 2^4
		2,  // 2^5
		3,  // 2^6
		3,  // 2^7
		3,  // 2^8
		4,  // 2^9
		4,  // 2^10
		4,  // 2^11
		4,  // 2^12
		5,  // 2^13
		5,  // 2^14
		5,  // 2^15
		6,  // 2^16
		6,  // 2^17
		6,  // 2^18
		7,  // 2^19
		7,  // 2^20
		7,  // 2^21
		7,  // 2^22
		8,  // 2^23
		8,  // 2^24
		8,  // 2^25
		8,  // 2^26
		9,  // 2^27
		9,  // 2^28
		9,  // 2^29
		10  // 2^30
	};

	static void DecodeWithLeading(uint32_t encoded, std::string& output)
	{
		size_t startingBit = 0;
		if (encoded != 0)
		{
			startingBit = HighestSetBitPosition(encoded);
			encoded &= ~(1 << startingBit);
		}

		const size_t length = lutBack[startingBit];
		const size_t offset = output.length();
		output.resize(offset + length);
		auto ptr = output.data() + offset;

		size_t index = 0;

		while (encoded > 0)
		{
			size_t oldValue = encoded;
			encoded /= ten;
			size_t mod = oldValue - encoded * ten;
			ptr[length - index - 1] = ('0' + static_cast<char>(mod));
			index++;
		}

		if (index < length)
		{
			std::memset(ptr, '0', length - index);
			index = length;
		}
	}

	uint64_t EncodeCodeId(LogLevel level, const std::string& input)
	{
		// example input: HX0001

		uint64_t prefix = 0;
		size_t i = 0;
		const auto length = input.length();

		for (; i < length; i++)
		{
			auto c = input[i];
			if (std::isalpha(c))
			{
				prefix = (prefix << 5) | static_cast<size_t>(std::toupper(c) - 'A' + 1);
			}
			else
			{
				break;
			}
		}
		prefix |= static_cast<uint64_t>(level < LogLevel_Info ? 0 : level - LogLevel_Info) << 30;
		return (prefix << 32) | EncodeWithLeading(input, i, length);
	}

	static std::string DecodeCodeId(uint64_t code)
	{
		uint32_t lower = code & 0xFFFFFFFF;
		uint32_t prefix = (code >> 32u) & ~(0x3u << 30u);

		std::string result;
		while (prefix)
		{
			size_t c = prefix & 0x1F;
			if (c == 0) break;
			result += 'A' + static_cast<char>(c - 1);
			prefix >>= 5;
		}
		std::reverse(result.begin(), result.end());
		DecodeWithLeading(lower, result);
		return result;
	}

	std::string GetStringForCode(uint64_t code)
	{
		return DecodeCodeId(code);
	}

	LogLevel GetLogLevelForCode(uint64_t code)
	{
		return static_cast<LogLevel>((code >> 62) + LogLevel_Info);
	}
}