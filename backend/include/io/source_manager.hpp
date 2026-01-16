#ifndef SOURCE_MANAGER_HPP
#define SOURCE_MANAGER_HPP

#include "source_location.hpp"
#include "source_file.hpp"
#include "text_stream.hpp"
#include "lexical/text_span.hpp"

namespace HXSL
{
	class SourceManager
	{
		std::vector<std::unique_ptr<SourceFile>> files;

	public:
		void AddSource(std::unique_ptr<SourceFile>&& file) { files.push_back(std::forward<std::unique_ptr<SourceFile>>(file)); };
		SourceFile* AddSource(Stream* stream, bool closeStream)
		{
			files.push_back(std::make_unique<SourceFile>(this, static_cast<SourceFileID>(files.size()), stream, closeStream));
			return files.back().get();
		}

		const SourceFile* GetSource(SourceFileID id) const { return files[id].get(); }

		std::string GetString(const TextSpan& span) const { return GetSource(span.source)->GetString(span.start, span.length); }

		StringSpan GetSpan(const TextSpan& span) const { return GetSource(span.source)->GetSpan(span.start, span.length); }
	};
}

#endif