import zstandard as zstd
import struct
from diag_message import DiagMessage

MAGIC_STRING = "TRANSL" 
CURRENT_VERSION = 1  

def write_translations(filename: str, messages: list[DiagMessage]):
    with open(filename, 'wb') as f:
        f.write(MAGIC_STRING.encode('utf-8'))
        f.write(struct.pack('I', CURRENT_VERSION))
        f.write(struct.pack('Q', len(messages)))

        cctx = zstd.ZstdCompressor(level=3)
        with cctx.stream_writer(f) as writer:
            for msg in messages:
                value_bytes = (msg.message or "").encode('utf-8') 
                str_length = struct.pack('I', len(value_bytes)) 
        
                writer.write(struct.pack('Q', msg.lookup_id.value))
                writer.write(str_length)
                writer.write(value_bytes)
            writer.flush(zstd.FLUSH_FRAME)