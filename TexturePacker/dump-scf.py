#!/usr/bin/env python3

import sys
import struct
import collections
import array
import PIL.Image
import io
import os.path
import itertools

def read_ascii(f):
    size = f.read(1)[0]
    if size == 255:
        return None
    else:
        return f.read(size).decode()

Matrix = collections.namedtuple('Matrix', ('a', 'b', 'c', 'd', 'tx', 'ty'))

ColorTransform = collections.namedtuple('ColorTransform',
                                        ('rm', 'gm', 'bm', 'am', 'ra', 'ga', 'ba'))

Shape = collections.namedtuple('Shape', ('id', 'commands'))

ShapeDrawBitmapCommand = collections.namedtuple('ShapeDrawBitmapCommand',
                                                ('texture', 'xys', 'uvs'))

TextField = collections.namedtuple('TextField', ('id',))

def parse_rgba4444_bitmap(data):
    # TODO: Not sure about the actual order.
    result = bytearray()
    for i in range(0, len(data), 2):
        result.append((data[i+1] >> 4) * 17)
        result.append((data[i+1] & 0xf) * 17)
        result.append((data[i] >> 4) * 17)
        result.append((data[i] & 0xf) * 17)
    return bytes(result)


def parse_rgba5551_bitmap(data):
    result = bytearray()
    for i in range(0, len(data), 2):
        result.append( (data[i+1] >> 3) * 9 )
        result.append( (((data[i+1] & 0x07) << 2 ) | (data[i] >> 6)) * 9 )
        result.append( ((data[i] >> 1) & 0x1f) * 9 )
        result.append( (data[i] & 0x1) << 4 )
    return bytes(result)

def parse_rgb565_bitmap(data):
    result = bytearray()
    for i in range(0, len(data), 2):
        result.append( (data[i+1] >> 3) * 7 )
        result.append( (((data[i+1] & 0x07) << 3 ) | (data[i] >> 5)) * 3 )
        result.append( (data[i] & 0x1f) * 7 )
        result.append( 255 )
    return bytes(result)

def parse_rgba8888_bitmap(data):
    return data

def parse_la88_bitmap(data):
     # TODO: Not sure about the actual order.
    result = bytearray()
    for i in range(0, len(data), 2):
        result.append((data[i+1]))
        result.append((data[i+1]))
        result.append((data[i+1]))
        result.append(0xff)
    return bytes(result)

BITMAP_PARSERS = [parse_rgba4444_bitmap,
                  parse_rgba5551_bitmap,
                  parse_rgb565_bitmap,
                  parse_rgba8888_bitmap,
                  parse_la88_bitmap]

attributes = [
    None,
    'texture',
    'shape',
    'movie_clip',
    None,
    None,
    None,
    'text_field',
    'matrix',
    'color_transform',
    'movie_clip',
    None,
    'movie_clip',
    'timeline_offset',
    'movie_clip',
    'text_field',
    'texture',
    'texture',
    'movie_clip',
]


class Scf:
    def __init__(self):
        self.textures = []
        self.movie_clips = []
        self.shapes = []
        self.color_transforms = []
        self.matrixs = []
        self.text_fields = []
        self.timeline_offsets = []


    def parse(self, f):
        f.read(12)  # These are the count of all objects, but we don't need these.
        f.read(5)   # These bytes are just ignored (why 5 bytes...?)

        export_count = int(struct.unpack('<H', f.read(2))[0])
        export_ids = array.array('H', f.read(2 * export_count))
        print(export_count)
        print (export_ids)
        export_names = [read_ascii(f) for _ in range(export_count)]
        self.exports = dict(zip(export_ids, export_names))
        self.export_names = export_names
        print(export_names)
        while True:
            (tag_type, tag_length) = struct.unpack('<BI', f.read(5))
            if tag_type == 0:
                break

            #if tag_type == 2:
             #   print("in shape")
            print('tag_type ',tag_type, '  ', attributes[tag_type])
            print('tag_length ',tag_length)
            data = f.read(tag_length)
           
            attribute = attributes[tag_type]
            result = getattr(self, 'parse_' + attribute)(data, tag_type)
            getattr(self, attributes[tag_type] + 's').append(result)


    @staticmethod
    def parse_matrix(data, tag_type):
        m = struct.unpack('<6i', data)
        return Matrix(a=m[0]/1024, b=m[1]/1024, c=m[2]/1024, d=m[3]/1024,
                      tx=m[4]/-20, ty=m[5]/-20)


    @staticmethod
    def parse_color_transform(data, tag_type):
        ct = struct.unpack('<7B', data)
        return ColorTransform(ra=ct[0], ga=ct[1], ba=ct[2],
                              am=ct[3], rm=ct[4], gm=ct[5], bm=ct[6])


    @staticmethod
    def parse_texture(data, tag_type):
        (sc_type, width, height) = struct.unpack_from('<BHH', data)
        if 2 <= sc_type < 7:
            sc_type -= 2
        else:
            sc_type = 3

        rgba_bytes = BITMAP_PARSERS[sc_type](data[5:])
        image = PIL.Image.frombytes('RGBA', (width, height), rgba_bytes)
        return image


    def parse_shape(self, data, tag_type):
        f = io.BytesIO(data)
        (id_, commands_count) = struct.unpack('<HH', f.read(4))

        commands = []
        for _ in range(commands_count):
            (command_type, length) = struct.unpack('<BI', f.read(5))
            shape_data = f.read(length)

            if command_type == 0:
                break
            elif command_type != 4:
                continue

            (tex_id, *rest) = struct.unpack('<B8I8h', shape_data)
            
            commands.append(ShapeDrawBitmapCommand(
                texture=self.textures[tex_id],
                xys=((rest[0]/-20, rest[1]/-20),
                
                     (rest[2]/-20, rest[3]/-20),
                     (rest[4]/-20, rest[5]/-20),
                     (rest[6]/-20, rest[7]/-20)),
                uvs=((rest[8], rest[9]),
                     (rest[10], rest[11]),
                     (rest[12], rest[13]),
                     (rest[14], rest[15]))
            ))

        return Shape(id=id_, commands=commands)


    @staticmethod
    def parse_text_field(data, tag_type):
        # We don't care about text fields.
        return TextField('')
        #return TextField(id=struct.unpack('<H', f.read(2))[0])
        #
        # f = io.BytesIO(data)
        # id_ = struct.unpack('<H', f.read(2))[0]
        # font_name = read_ascii(f)
        # (color, font_prop_2, font_prop_3, is_multiline, x90, align,
        #  font_size, x9c, xa0, xa4, xa8, font_prop_1) = struct.unpack('<I4?2B4H?', f.read(19))
        # text = read_ascii(f)
        # return (id_, font_name, color, font_prop_2, font_prop_3, is_multiline,
        #         x90, align, font_size, x9c, xa0, xa4, xa8, font_prop_1, text)

    #almost copy the pharse shape method
    def parse_movie_clip(self, data, tag_type):
        f = io.BytesIO(data)
        (id_, commands_count) = struct.unpack('<HH', f.read(4))

        commands = []
        if commands_count != 1:
            return Shape(id=-1,commands=commands)
        for _ in range(commands_count):
            (vertex_count,unknow_2byte, length) = struct.unpack('<BHI', f.read(7))
            #unknow_2byte always 0011,4352
            shape_data = f.read(length)

            # msg = 'unknow_2byte = {0}'.format(unknow_2byte)
            # print(msg)

            if vertex_count == 0:
                break
            #elif vertex_count != 4:
            #    msg = 'skipped vertex_count {0}'.format(vertex_count)
            #    print(msg)
            #    continue

            fmt = '<BB{0}I{0}h'.format(vertex_count*2)
            (tex_id,vertex_count_again, *rest) = struct.unpack(fmt, shape_data)

            #(tex_id,vertex_count_again, *rest) = struct.unpack('<BB8I8h', shape_data)

            coord_xy = []
            coord_uv = []
            for _1 in range(vertex_count):
                coord_xy.append((rest[_1*2]/-20, rest[_1*2+1]/-20))
                coord_uv.append((rest[_1*2 + vertex_count*2], rest[_1*2 + 1 + vertex_count*2]))

            if vertex_count != 4:
                print('shape with {0} vertex'.format(vertex_count))
                for _1 in range(vertex_count):
                    print('({0}, {1})'.format(rest[_1*2 + vertex_count*2], rest[_1*2 + 1 + vertex_count*2]))

            #if (tex_id == 0):
            commands.append(ShapeDrawBitmapCommand(
                texture=self.textures[tex_id],
                xys = coord_xy,
                uvs = coord_uv
            ))
            
            
            #commands.append(ShapeDrawBitmapCommand(
            #    texture=self.textures[tex_id],
            #    xys=((rest[0]/-20, rest[1]/-20),
            #         (rest[2]/-20, rest[3]/-20),
            #         (rest[4]/-20, rest[5]/-20),
            #         (rest[6]/-20, rest[7]/-20)),
            #    uvs=((rest[8], rest[9]),
            #         (rest[10], rest[11]),
            #         (rest[12], rest[13]),
            #         (rest[14], rest[15]))
            #))
        return Shape(id=id_, commands=commands)

def isLeft(p0, p1, p2):
    area2 = (p1[0] - p0[0]) * (p2[1] - p0[1]) - (p2[0] - p0[0]) * (p1[1] - p0[1])
    return area2 < 0

def point_inside_polygon(x,y,poly):

    n = len(poly)

    res = True
    
    for i in range(n):
        pt0 = poly[i]
        pt1 = poly[(i+1) % n]
        if isLeft(pt0, pt1, (x, y)):
            res = False
            break       
    return res

#if len(sys.argv) < 3:
#    print('Usage: ./dump-scf.py [*.scf] [directory]')
#    exit(0)


def unpack_file(src_dir, src_file):
    scf = Scf()

    directory = src_dir + '\\Result\\' + src_file
    
    if not os.path.exists(directory):
        os.makedirs(directory)
    
    exported_regions = set()
    with open(os.path.join(src_dir, src_file), 'rb') as f:
        scf.parse(f)
        print('here')
        print(len(scf.shapes))
        for shape in scf.shapes:
            shape_id = shape.id
            for index, command in enumerate(shape.commands):
                uvs = command.uvs
                min_u = min(uv[0] for uv in uvs)
                max_u = max(uv[0] for uv in uvs)
                min_v = min(uv[1] for uv in uvs)
                max_v = max(uv[1] for uv in uvs)
                if min_u == max_u or min_v == max_v:
                    continue
                export_region = (min_u, min_v, max_u, max_v)
                key = (id(command.texture), export_region)
                if key in exported_regions:
                    continue
                exported_regions.add(key)
                image = command.texture.crop(export_region)
                name = '{0}.{1}.png'.format(shape_id, index)
                image.save(os.path.join(directory, name))
        index = 0
        for texture in scf.textures:
            texture_name = '{0}.png'.format(index)
            texture.save(os.path.join(directory, texture_name))
            index=index+1

        print('Movie clip num')
        print(len(scf.movie_clips))
        for movie_clip in scf.movie_clips:
            movie_clip_id = movie_clip.id
            if movie_clip_id == -1:
                continue
            for index, command in enumerate(movie_clip.commands):
                uvs = command.uvs
                min_u = min(uv[0] for uv in uvs)
                max_u = max(uv[0] for uv in uvs)
                min_v = min(uv[1] for uv in uvs)
                max_v = max(uv[1] for uv in uvs)
                
                
                
                if min_u == max_u or min_v == max_v:
                    print('{0}.{1}.png has size 0, skipped.'.format(movie_clip_id, index) )
                    continue
                export_region = (min_u, min_v, max_u, max_v)
                key = (id(command.texture), export_region)
                #if key in exported_regions:
                #    print('{0}.{1}.png key has already been exported, skipped.'.format(movie_clip_id, index) )
                #    continue
                
                exported_regions.add(key)
                image = command.texture.crop(export_region)
                clip_name = scf.exports.get(movie_clip_id,-1)
                print('movie_clip_id',movie_clip_id)
                if clip_name != -1:
                    name = '{0}.{1}.png'.format(clip_name, index)
                else:
                    name = '{0}.{1}.png'.format(movie_clip_id, index)
                    
                if len(uvs) != 4:
                    for x in range(image.size[0]):
                        for y in range(image.size[1]):
                            if not point_inside_polygon(x+min_u, y+min_v, uvs):
                                image.putpixel((x,y), 0)
                    
                image.save(os.path.join(directory, name))


files = ['clouds', 'in-game', 'in-game_lowres', 'loading', 'map', 'map_lowres', 'ui', 'ui_lowres', 'units','units_lowres']
files = ['units','units_lowres']
fileDir = 'D:\\boombeach unpack\\4-28\\'

files = ['in-game']

fileDir = 'C:\\dev\\old things\\in-game'

for f in files:
    unpack_file(fileDir, f)

# dump-scf.py --- Dump images from decompressed SC files.
# Copyright (C) 2013  kennytm
#
# This program is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with
# this program.  If not, see <http://www.gnu.org/licenses/>.

