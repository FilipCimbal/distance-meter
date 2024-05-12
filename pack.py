#!/usr/bin/env python3

import logging
import os
import os.path
import click
import gzip
import io

def template(output, archive):
    output.write(
        "#include \"inline_web.h\"\n"
        "#include <unordered_map>\n"
        "const uint8_t blob[] = {\n"
    )
    for file in archive:
        output.write(
        f"// {file['name']}\n"
        )
        for i in range(0, file["size"], 16):
            output.write("    ")
            for n in range(i, min(i+16, file["size"])):
                output.write(f"{file['data'][n]:3}, ")
            output.write("\n")
    output.write(
        "};\n\n"
    )

    output.write(
        "const std::unordered_map<std::string, EmbededFile> file_map = {\n"
    )

    offset = 0
    for file in archive:
        output.write(
        f"      {{\"{file['name']}\", {{{file['size']}, blob+{offset}}}}},\n"
        )
        offset += file["size"]
    output.write(
        "};\n\n"
    )

    output.write(
        "const EmbededFile& getEmbededFile(std::string path){\n"
        "   if(path == \"/\") path = \"/index.html\";\n"
        "   return file_map.at(path);\n"
        "}\n"
        "\n"
        "const EmbededFiles& getEmbededFiles(){\n"
        "   return file_map;\n"
        "}\n"
    )

def pack(path, name, files):
    print(f"Packing {path} as {name}")
    with open(path, "rb") as file:
        raw_data = file.read()
        gziped_data = gzip.compress(raw_data)
    files.append({
        "name": name,
        "data": gziped_data,
        "size": len(gziped_data)
    })
    return len(raw_data), len(gziped_data)

@click.command()
@click.option("--skip", multiple=True, default=["map"])
@click.option("--add", multiple=True, default=["html", "js", "css"])
@click.option("-o", "--output", default="include/web_content.h")
def main(skip, output, add):
    web_path = "web/dist"

    print("Web path:", web_path)
    print("Result file:", output)

    total_raw = 0
    total_gz = 0

    archive=[]

    for root, dirs, files in os.walk(web_path):
        for filename in files:
            extension = os.path.splitext(filename)[1][1:].strip()
            if extension in skip:
                continue
            if extension not in add:
                continue
            path = os.path.join(root, filename)
            name = path[len(os.path.commonpath((web_path, path))):]
            r, g = pack(path, name, archive)
            total_raw += r
            total_gz += g

    print("Packing result:")
    print(f" Raw size: {total_raw/1024:.2f} kB")
    print(f" Gzipped size: {total_gz/1024:.2f} kB")

    with open(output, "w") as output_file:
        template(output_file, archive)

if __name__=="__main__":
    main()