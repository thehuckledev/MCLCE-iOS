#!/usr/bin/env python3
import struct, sys, argparse
import xml.etree.ElementTree as ET

TAG_END = 0
TAG_BYTE = 1
TAG_SHORT = 2
TAG_INT = 3
TAG_LONG = 4
TAG_FLOAT = 5
TAG_DOUBLE = 6
TAG_BYTE_ARRAY = 7
TAG_STRING = 8
TAG_LIST = 9
TAG_COMPOUND = 10
TAG_INT_ARRAY = 11

class NBTParser:
    def __init__(self, filename, endian='>'):
        self.stream = open(filename, 'rb')
        self.endian = endian

    def read(self, fmt):
        size = struct.calcsize(fmt)
        data = self.stream.read(size)
        if len(data) != size:
            raise EOFError("Unexpected end of file")
        return struct.unpack(fmt, data)[0]

    def read_byte(self):
        return self.read(self.endian + 'b')
    def read_unsigned_byte(self):
        return self.read(self.endian + 'B')
    def read_short(self):
        return self.read(self.endian + 'h')
    def read_int(self):
        return self.read(self.endian + 'i')
    def read_long(self):
        return self.read(self.endian + 'q')
    def read_float(self):
        return self.read(self.endian + 'f')
    def read_double(self):
        return self.read(self.endian + 'd')

    def read_string(self):
        length = self.read(self.endian + 'h')  # unsigned short
        data = self.stream.read(length)
        return data.decode('utf-8')

    def read_byte_array(self):
        length = self.read(self.endian + 'i')
        return self.stream.read(length)

    def read_int_array(self):
        length = self.read(self.endian + 'i')
        return [self.read_int() for _ in range(length)]

    def read_list(self):
        elem_type = self.read_unsigned_byte()
        count = self.read(self.endian + 'i')
        return [self.read_tag_payload(elem_type) for _ in range(count)]

    def read_compound(self):
        compound = {}
        while True:
            tag_type = self.read_unsigned_byte()
            if tag_type == TAG_END:
                break
            name = self.read_string()
            compound[name] = self.read_tag_payload(tag_type)
        return compound

    def read_tag_payload(self, tag_type):
        if tag_type == TAG_BYTE:
            return self.read_byte()
        elif tag_type == TAG_SHORT:
            return self.read_short()
        elif tag_type == TAG_INT:
            return self.read_int()
        elif tag_type == TAG_LONG:
            return self.read_long()
        elif tag_type == TAG_FLOAT:
            return self.read_float()
        elif tag_type == TAG_DOUBLE:
            return self.read_double()
        elif tag_type == TAG_BYTE_ARRAY:
            return self.read_byte_array()
        elif tag_type == TAG_STRING:
            return self.read_string()
        elif tag_type == TAG_LIST:
            return self.read_list()
        elif tag_type == TAG_COMPOUND:
            return self.read_compound()
        elif tag_type == TAG_INT_ARRAY:
            return self.read_int_array()
        else:
            raise ValueError(f"Unknown tag type: {tag_type}")

    def parse(self):
        root_tag = self.read_unsigned_byte()
        if root_tag != TAG_COMPOUND:
            raise ValueError("Expected root TAG_Compound (0x0A)")
        _ = self.read_string()  # root name
        data = self.read_compound()
        self.stream.close()
        return data

def structure_to_xml(data, xml_root_name="Structure"):
    """convert nbt data into xml"""
    root = ET.Element(xml_root_name)
    root.set("author", str(data.get("author", "Unknown")))
    root.set("version", str(data.get("version", "")))
    # Size
    size = data.get("size", [0,0,0])
    ET.SubElement(root, "Size", {
        "width": str(size[0]), "height": str(size[1]), "length": str(size[2])
    })

    # Palette
    pal_elem = ET.SubElement(root, "Palette")
    palette = data.get("palette", [])
    for i, entry in enumerate(palette):
        attrs = {"index": str(i)}
        if "Name" in entry:
            attrs["name"] = entry["Name"]
        block_elem = ET.SubElement(pal_elem, "Block", attrs)
        # add properties as subelements
        props = entry.get("Properties", {})
        for prop_name, prop_value in props.items():
            prop_elem = ET.SubElement(block_elem, "Property")
            prop_elem.set("name", prop_name)
            prop_elem.text = str(prop_value)

    # Blocks
    blocks_elem = ET.SubElement(root, "Blocks")
    for blk in data.get("blocks", []):
        pos = blk.get("pos", [])
        state = blk.get("state", 0)
        if pos and len(pos) == 3:
            attrs = {"x": str(pos[0]), "y": str(pos[1]), "z": str(pos[2]),
                     "index": str(state)}
            ET.SubElement(blocks_elem, "Block", attrs)

    # Entities
    ents = data.get("entities", [])
    ents_elem = ET.SubElement(root, "Entities")
    for ent in ents:
        ent_elem = ET.SubElement(ents_elem, "Entity")
        for key, val in ent.items():
            sub = ET.SubElement(ent_elem, key)
            sub.text = str(val)
    return root

def main():
    parser = argparse.ArgumentParser(description="Convert NBT structure files to XML")
    parser.add_argument("inputs", nargs="+", help="Input NBT file(s)")
    parser.add_argument("-l", "--little", action="store_true",
                        help="Assume little-endian NBT format")
    parser.add_argument("-o", "--output", help="Output XML file (or directory)")
    args = parser.parse_args()

    endian = '<' if args.little else '>'
    for infile in args.inputs:
        try:
            parser_nbt = NBTParser(infile, endian=endian)
            data = parser_nbt.parse()
        except Exception as e:
            print(f"Error parsing {infile}: {e}", file=sys.stderr)
            continue

        xml_root = structure_to_xml(data)
        tree = ET.ElementTree(xml_root)
        outpath = args.output or infile + ".xml"
        if args.output and len(args.inputs) > 1:
            import os
            os.makedirs(args.output, exist_ok=True)
            base = os.path.basename(infile)
            outpath = os.path.join(args.output, base + ".xml")
        ET.indent(tree, space="    ", level=0)
        tree.write(outpath, encoding="utf-8", xml_declaration=True)
        print(f"Wrote XML to {outpath}")

if __name__ == "__main__":
    main()
