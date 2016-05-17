import sys
import struct
import pdb
import array
from leb128 import *
from dex_class import *

DEX_MAGIC = "dex\n"
DEX_OPT_MAGIC = "dey\n"


class dex_parser:
    """docstring for dex_parser"""

    def __init__(self, file_path):
        global DEX_MAGIC
        global DEX_OPT_MAGIC

        self.m_file_path = file_path
        self.m_fd = open(file_path, "rb")
        self.m_content = self.m_fd.read()
        self.m_fd.close()

        self.m_dex_optheader = None
        self.m_class_name_id = {}  # dirct     class_name : index
        self.m_string_table = []  # list

        if self.m_content[0:4] == DEX_OPT_MAGIC:
            # self.init_optheader()
            pass
        elif self.m_content[0:4] == DEX_MAGIC:
            self.init_header(self.m_content, 0)
        bOffset = self.m_stringIdsoff
        if self.m_stringIdsSize > 0:
            for i in xrange(0, self.m_stringIdsSize):
                # pdb.set_trace()
                offset, = struct.unpack_from(
                    "I", self.m_content, bOffset + i * 4)
                start = offset
                skip, length = get_uleb128(self.m_content[start:start + 5])
                # self.string_table.append(self.m_content[start+skip:offset-1])
                self.m_string_table.append(
                    self.m_content[start + skip: start + skip + length])

            for x in xrange(0, self.m_classDefSize):
                str1 = self.get_classname(x)
                self.m_class_name_id[str1] = x
                '''
                str1  'Lcom/example/inotify/BuildConfig;'
                        'Lcom/example/inotify/MainActivity$1;'
                        ......
                '''

            for i in xrange(0, self.m_classDefSize):
                dex_class(self, i) . printf(self)

    def create_all_headers(self):
        # traverse all class in class_def_item
        for x in xrange(0, self.m_classDefSize):
            str1 = self.get_classname(x)
            pdb.set_trace()
            self.create_cpp_header(str1)

    def create_cpp_header(self, class_name="Landroid/app/Activity;"):
        # if self.m_class_name_id.in(class_name):
        # index into typeIds for this class
        if self.m_class_name_id.has_key(class_name):
            class_idx = self.m_class_name_id[class_name]
            field_list = dex_class(
                self, class_idx).create_header_file_cplusplus(self)

    def get_typename(self, type_id):
        if type_id >= self.m_typeIdsSize:
            return ""
        offset = self.m_typeIdsOff + type_id * struct.calcsize("I")
        descriptorIdx, = struct.unpack_from("I", self.m_content, offset)
        return self.m_string_table[descriptorIdx]

    def get_classname(self, class_id):
        if class_id >= self.m_classDefSize:
            return ""

        offset = self.m_classDefOff + class_id * struct.calcsize("8I")
        # DexClassDef
        '''
        class_idx, access_flags, superclass_idx, interfaces_off, source_file,
        annotations_off, class_dataoff, static_value_off, = struct.unpack_from(
            "8I", self.m_content, offset)
        '''

        class_idx, access_flags, superclass_idx, interfaces_off, source_file, \
            annotations_off, class_data_off, static_value_off, = struct.unpack_from(
                "8I", self.m_content, offset)

        return self.get_typename(class_idx)

    def init_header(self, content, offset):

        format = "4s"
        self.m_magic = struct.unpack_from(format, content, offset)
        offset += struct.calcsize(format)

        format = "I"
        self.m_version = struct.unpack_from(format, content, offset)
        offset += struct.calcsize(format)
        self.m_checksum = struct.unpack_from(format, content, offset)
        offset += struct.calcsize(format)
        format = "20s"
        self.m_signature = struct.unpack_from(format, content, offset)
        offset += struct.calcsize(format)

        format = "I"
        self.m_filesize, = struct.unpack_from(format, content, offset)
        offset += struct.calcsize(format)
        self.m_headerSize, = struct.unpack_from(format, content, offset)
        offset += struct.calcsize(format)
        self.m_endianTag, = struct.unpack_from(format, content, offset)
        offset += struct.calcsize(format)
        self.m_linkSize, = struct.unpack_from(format, content, offset)
        offset += struct.calcsize(format)
        self.m_linkOff, = struct.unpack_from(format, content, offset)
        offset += struct.calcsize(format)
        self.m_mapOff, = struct.unpack_from(format, content, offset)
        offset += struct.calcsize(format)
        self.m_stringIdsSize, = struct.unpack_from(format, content, offset)
        offset += struct.calcsize(format)
        self.m_stringIdsoff, = struct.unpack_from(format, content, offset)
        offset += struct.calcsize(format)
        self.m_typeIdsSize, = struct.unpack_from(format, content, offset)
        offset += struct.calcsize(format)
        self.m_typeIdsOff, = struct.unpack_from(format, content, offset)
        offset += struct.calcsize(format)
        self.m_protoIdsSize, = struct.unpack_from(format, content, offset)
        offset += struct.calcsize(format)
        self.m_protoIdsOff, = struct.unpack_from(format, content, offset)
        offset += struct.calcsize(format)
        self.m_fieldIdsSize, = struct.unpack_from(format, content, offset)
        offset += struct.calcsize(format)
        self.m_fieldIdsOff, = struct.unpack_from(format, content, offset)
        offset += struct.calcsize(format)
        self.m_methodIdsSize, = struct.unpack_from(format, content, offset)
        offset += struct.calcsize(format)
        self.m_methodIdsOff, = struct.unpack_from(format, content, offset)
        offset += struct.calcsize(format)
        self.m_classDefSize, = struct.unpack_from(format, content, offset)
        offset += struct.calcsize(format)
        self.m_classDefOff, = struct.unpack_from(format, content, offset)
        offset += struct.calcsize(format)
        self.m_dataSize, = struct.unpack_from(format, content, offset)
        offset += struct.calcsize(format)
        self.m_dataOff, = struct.unpack_from(format, content, offset)
        offset += struct.calcsize(format)

    def get_access_flags(self, access_flags):
        val = {
            1: "public",
            2: "private",
            4: "protected",
            8: "static",
            0x10: "final",
            0x20: "synchronized",
            0x40: "volatile",
            0x80: "bridge",
            0x100: "native",
            0x200: "interface",
            0x400: "abstract",
            0x800: "strict",
            0x1000: "synthetic",
            0x2000: "annotation",
            0x4000: "enum",
            0x8000: "unused",
            0x10000: "constructor",
            0x20000: "declared_synchronized"
        }
        value = ""
        bFirst = True
        for key in val:
            if access_flags & key:
                if bFirst == True:
                    value += ""
                    bFirst = False
                else:
                    value += " "
                value += val[key]

        if value == "":
            value += "public"

        return value

    def get_field_fullname(self, field_idx):
            # DexFieldId
        if field_idx >= self.m_fieldIdsSize:
            return ""
        offset = self.m_fieldIdsOff + field_idx * struct.calcsize("HHI")
        '''
         struct DexFieldId {
            u2  classIdx;           /* index into typeIds list for defining class */
            u2  typeIdx;           /* index into typeIds for field type */
            u4  nameIdx;         /* index into stringIds for field name */
        };
        '''
        class_idx, type_idx, name_idx, = struct.unpack_from(
            "HHI", self.m_content, offset)
        type_sign = self.get_typename(type_idx)  # sign
        type_str = decode_signature(type_sign)
        name_str = self.m_string_table[name_idx]

        return " " + type_str + " " + name_str

    def get_method_fullname(self, method_idx):
        pass


def decode_signature(sign):
    val = {
        "V": "void",
        "Z": "boolean",
        "B": "byte",
        "S": "short",
        "C": "char",
        "I": "int",
        "J": "long",
        "F": "float",
        "D": "double",
        "L": "L"
    }
    value = ""

    if sign[-1] == ":":
        if sign[0] == 'L':
            return sign[1: -1].repalce("/", ".")
        if sign[0] == '[':
            if sign[1] == 'L':
                return sign[2: -1].repalce("/", ".") + "[]"
            else:
                return sign[1: -1] + "[]"

    value = ""
    for ch in sign:
        if val.has_key(ch):
            value += val[ch]

    if '[' in sign:
        value += "[]"

    return value


def main():
    pdb.set_trace()
    if len(sys.argv) < 2:
        print "Usages: %s dex file" % sys.argv[0]
        quit()
    file_path = sys.argv[1]
    dex = dex_parser(file_path)
    dex.create_all_headers()


if __name__ == '__main__':
    main()
