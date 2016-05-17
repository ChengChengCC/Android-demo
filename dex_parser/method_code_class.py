import pdb
import struct
from leb128 import *


class method_code_class:

    def __init__(self, dex_object, offset):
        '''
        /*
        * Direct-mapped "code_item".
        *
        * The "catches" table is used when throwing an exception,
        * "debugInfo" is used when displaying an exception stack trace or
        * debugging. An offset of zero indicates that there are no entries.
        */
       struct DexCode {
            u2  registersSize;
            u2  insSize;
            u2  outsSize;
            u2  triesSize;
            u4  debugInfoOff;       /* file offset to debug info stream */
            u4  insnsSize;          /* size of the insns array, in u2 units */
            u2  insns[1];
            /* followed by optional u2 padding */
            /* followed by try_item[triesSize] */
            /* followed by uleb128 handlersSize */
            /* followed by catch_handler_item[handlersSize] */
        };
        '''
        format = "H"
        self.m_registers_size = struct.unpack_from(
            format, dex_object.m_content, offset)
        offset += struct.calcsize(format)

        self.m_ins_size = struct.unpack_from(
            format, dex_object.m_content, offset)
        offset += struct.calcsize(format)

        self.m_outs_size = struct.unpack_from(
            format, dex_object.m_content, offset)
        offset += struct.calcsize(format)

        self.m_tries_size = struct.unpack_from(
            format, dex_object.m_content, offset)
        offset += struct.calcsize(format)

        format = "I"
        self.m_debug_info_off = struct.unpack_from(
            format, dex_object.m_content, offset)
        offset += struct.calcsize(format)

        self.m_insns_size = struct.unpack_from(
            format, dex_object.m_content, offset)
        offset += struct.calcsize(format)

        self.m_insns_array_off = offset
        offset += self.m_insns_size * 2  # insns

        if self.m_tries_size % 2 == 1:
            offset += 2  # u2 padding

        if self.m_tries_size == 0:
            self.m_try_item = 0
            skip, self.m_handlers_size = get_uleb128(
                dex_object.m_content[offset:])
            offset += skip
            self.m_catch_handlers_item = offset
        else:
            self.m_try_item = offset
            # try_item
            offset += self.m_tries_size * struct.calcsize("IHH")  # IHH  DexTry
            skip, self.m_handlers_size = get_uleb128(
                dex_object.m_content[offset:])
            offset += skip
            self.m_catch_handlers_item = offset

    def get_parameter_list(self, dex_object):
        if self.m_debug_info_off != 0:
            pass
