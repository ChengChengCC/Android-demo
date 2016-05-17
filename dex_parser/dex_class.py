import pdb
import struct
from leb128 import *
from dex_code_class import *


class dex_class:

    def __init__(self, dex_object, class_index):
        # pdb.set_trace()
        if class_index >= dex_object.m_classDefSize:
            return ""
        '''

        '''
        offset = dex_object.m_classDefOff + class_index * struct.calcsize("8I")
        self.offset = offset
        format = "I"
        # class_index : index to class_def_item

        # index to typeIds for this class
        self.m_class_idx, = struct.unpack_from(
            format, dex_object.m_content, offset)
        offset += struct.calcsize(format)
        #
        self.m_access_flags, = struct.unpack_from(
            format, dex_object.m_content, offset)
        offset += struct.calcsize(format)
        # index into typeIds for superclass
        self.m_super_class_idx, = struct.unpack_from(
            format, dex_object.m_content, offset)
        offset += struct.calcsize(format)
        # file offset to DexTypeList
        self.m_interfaces_off, = struct.unpack_from(
            format, dex_object.m_content, offset)
        offset += struct.calcsize(format)
        # index into stringIds for source file name
        self.m_source_file_idx, = struct.unpack_from(
            format, dex_object.m_content, offset)
        offset += struct.calcsize(format)
        # file offset to annotation_directory_name
        self.m_annotations_off, = struct.unpack_from(
            format, dex_object.m_content, offset)
        offset += struct.calcsize(format)
        # file offset to class_data_item
        self.m_class_data_off, = struct.unpack_from(
            format, dex_object.m_content, offset)
        offset += struct.calcsize(format)
        # file offset to DexEncodeArray
        self.m_static_value_off, = struct.unpack_from(
            format, dex_object.m_content, offset)
        offset += struct.calcsize(format)

        self.m_index = class_index  # index to class_def_item
        if self.m_interfaces_off != 0:
            # DexTypeList size
            self.m_interfaces_size = struct.unpack_from(
                "I", dex_object.m_content, self.m_interfaces_off)
        if self.m_class_data_off != 0:

            offset = self.m_class_data_off  # offset ---> DexclassData
            # count , num = count,self.numStaticFields = get_uleb128(dex_object.m_content[offset:])
            skip, self.m_static_field_size, = get_uleb128(
                dex_object.m_content[offset:])
            # pdb.set_trace()
            offset += skip

            skip, self.m_instance_fields_size, = get_uleb128(
                dex_object.m_content[offset:])
            offset += skip

            skip, self.m_direct_methods_size, = get_uleb128(
                dex_object.m_content[offset:])
            offset += skip

            skip, self.m_virtual_methods_size, = get_uleb128(
                dex_object.m_content[offset:])
        else:
            self.m_static_field_size = 0
            self.m_instance_fields_size = 0
            self.m_direct_methods_size = 0
            self.m_virtual_methods_size = 0

    def printf(self, dex_object):
        pass

    def format_classname(self, name):
        name = name[1: -1].replace("/", "_")
        name = name.replace("$", "_")
        return name

    def create_header_file_cplusplus(self, dex_object):
        field_list = []
        name = self.format_classname(dex_object.get_typename(self.m_class_idx))
        f = open(name + ".smail", "w")
        str1 = "class %s" % name
        super_name = dex_object.get_typename(self.m_super_class_idx)

        if dex_object.m_class_name_id.has_key(super_name):
            str1 += ":"
            str1 += self.format_classname(super_name)
        str1 += "\n{\n"

        offset = self.m_class_data_off  # ---> DexClassData

        # staticFieldsSize
        skip, tmp_val, = get_uleb128(dex_object.m_content[offset:])
        offset += skip

        # instanceFieldsSize
        skip, tmp_val, = get_uleb128(dex_object.m_content[offset:])
        offset += skip

        # directMethodsSize
        skip, tmp_val = get_uleb128(dex_object.m_content[offset:])
        offset += skip

        # virtualMethodsSize
        skip, tmp_val = get_uleb128(dex_object.m_content[offset:])
        offset += skip

        # traverse DexField  staticFields
        for x in xrange(0, self.m_static_field_size):
            # field_idx : index to a field_id_item
            skip, field_idx = get_uleb128(dex_object.m_content[offset:])
            offset += skip

            skip, access_flags = get_uleb128(dex_object.m_content[offset:])
            offset += skip

            access_str = dex_object.get_access_flags(access_flags)
            str1 += access_str
            str1 += dex_object.get_field_fullname(field_idx)
            str1 += ";\n"
        if field_idx not in field_list:
            field_list.append(field_idx)

        # travser instance fields  DexField
        for x in xrange(0, self.m_instance_fields_size):
            skip, field_idx = get_uleb128(dex_object.m_content[offset:])
            offset += skip
            skip, access_flags = get_uleb128(dex_object.m_content[offset:])
            offset += skip
            access_str = dex_object.get_access_flags(access_flags)
            str1 += access_str
            str1 += dex_object.get_field_fullname(field_idx)
            str1 += ";\n"

        '''
            /* expanded form of encoded_method */
            struct DexMethod {
                u4 methodIdx;    /* index to a method_id_item */
                u4 accessFlags;
                u4 codeOff;      /* file offset to a code_item */
            };
        '''
        for x in xrange(0, self.m_direct_methods_size):
            skip, method_idx = get_uleb128(dex_object.m_content[offset:])
            offset += skip

            skip, access_flags = get_uleb128(dex_object.m_content[offset:])
            access_str = dex_object.get_access_flags(access_flags)
            offset += skip

            skip, code_off = get_uleb128(dex_object.m.m_content[offset:])
            offset += skip

            dex_code_class(dex_object, code_off)
