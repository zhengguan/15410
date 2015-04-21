from optparse import OptionParser
import os
from struct import pack
import random

SECTOR_SIZE = 512
MAX_EXECNAME_LENGTH = 256
EXEC_INFO_SIZE = 256

# Chosen to match the normal kernel infrastructure.
MAX_USER_APPS = 128

MAGIC_CONSTANT = 0xde001337

class UserFile(object):

    def __init__(self, name, path):
        self._name = name
        self.file_path = path
        self.file_name = os.path.basename(path)
        self.file_size = os.path.getsize(path)

        self.bytes_written = 0

        f = open(self.file_path)
        self.data = f.read()
        f.close()

    def name(self):
        return self._name

    def size(self):
        return self.file_size

    def bytes_remaining(self):
        return self.file_size - self.bytes_written

    def open(self):
        pass

    def get_bytes(self, num_bytes):
        if num_bytes <= self.bytes_remaining():
            res = self.data[self.bytes_written:(self.bytes_written + num_bytes)]
            self.bytes_written += num_bytes
            return res

        padding = '\0' * (num_bytes - self.bytes_remaining())
        res = self.data[self.bytes_written:]
        self.bytes_written = self.file_size
        return res + padding

    def close(self):
        pass

def build_argument_parser():
    parser = OptionParser(usage="usage: %prog [options] programs...")
    parser.add_option("-d", "--search_dir", dest="search_directory",
                      help="the directory in which programs will be located.")
    parser.add_option("-o", "--out", dest="output", default="hd.img",
                      help="the output file. Leave blank to write to %default.")
    parser.add_option("-n", "--num_sectors", dest="num_sectors", default=634966,
                      help="the number of sectors the output should have. "
                      "The default is %default sectors.")
    parser.add_option("-s", "--skip", dest="initial_offset",
                      default=1, help="the initial offset to write at in "
                      "sectors. The default is %default.")
    return parser

"""
Build a layout plan for the filesystem.

Builds a description of the disk layout. Since many pieces of the filesystem
have pointers to other pieces, we do this in several steps.
  1) Build a list of every element the filesystem will contain.
  2) Shuffle the list.
  3) Add a superblock to the front, and a final free node to the back.
  4) Calculate offsets.
  5) Add the necessary pointers.

"""
def build_plan(files, size):

    last_free = {
        'type': 'free',
        'next': None,
        '_': 'The last free block'
        }

    plan = []
    next_free = last_free
    next_file = None

    for f in files:
        sectors_to_write = f.size() / SECTOR_SIZE
        if (f.size() % SECTOR_SIZE) > 0:
            sectors_to_write += 1

        next_chunk = None
        _i = 0
        while sectors_to_write > 0:
            if sectors_to_write >= 8:
                chunk_size = random.randint(8, sectors_to_write)
            else:
                chunk_size = sectors_to_write

            chunk = {
                'type': 'chunk',
                'size': chunk_size,
                '_': 'Chunk {0} of {1}'.format(_i, f.name())
                }
            plan.append(chunk)

            chunk_node = {
                'type': 'chunk_node',
                'next': next_chunk,
                'chunk_size': chunk_size,
                'chunk': chunk,
                '_': 'Chunk node for chunk {0} of {1}'.format(_i, f.name())
                }
            next_chunk = chunk_node
            plan.append(chunk_node)

            sectors_to_write -= chunk_size
            _i += 1


        header = {
            'type': 'header',
            'next': next_file,
            'file': f,
            'is_writeable': True,  # Where does this come from?
            'first_chunk': next_chunk,
            '_': 'File node for {0}'.format(f.name())
            }
        next_file = header
        plan.append(header)

        s = random.randint(1, 4)
        free = {
            'type': 'free',
            'next': next_free,
            'size': s,
            '_': '{0} sectors of free space'.format(s)
            }
        next_free = free
        plan.append(free)

    # Now things get exciting.
    random.shuffle(plan)

    # Add the superblock. This happens after the shuffle since it must be at the
    # beginning.
    plan = [{
            'type': 'superblock',
            '_': 'superblock'
            }] + plan

    # Add offsets.
    offset = 0
    for item in plan:
        item['offset'] = offset
        try:
            offset += item['size']
        except KeyError:
            offset += 1

    last_free['offset'] = offset
    last_free['size'] = (size - offset) - 1
    plan.append(last_free)
    offset += 1

    if offset >= size:
        raise ValueError('Filesystem is too big for the disk.')

    def get_offset(x):
        if x is None:
            return 0
        else:
            return x['offset']

    # Add pointers.
    for item in plan:
        if item['type'] == 'superblock':
            item['first_file_pointer'] = get_offset(next_file)
            item['first_free_pointer'] = get_offset( next_free)
        elif item['type'] == 'free':
            item['next_pointer'] = get_offset(item['next'])
        elif item['type'] == 'header':
            item['next_pointer'] = get_offset(item['next'])
            item['first_chunk_pointer'] = get_offset(item['first_chunk'])
        elif item['type'] == 'chunk_node':
            item['next_pointer'] = get_offset(item['next'])
            item['chunk_pointer'] = get_offset(item['chunk'])
        elif item['type'] == 'chunk':
            pass

    return plan

def execute_plan(plan, out):

    def read_file_in(f, chunk_node):
        f.open()

        data = f.get_bytes(chunk_node['chunk_size'] * SECTOR_SIZE)

        chunk_node['chunk']['data'] = data
        next = chunk_node['next']


        if next is None:
            f.close()
        else:
            read_file_in(f, next)

    for item in plan:
        if item['type'] == 'header':
            read_file_in(item['file'], item['first_chunk'])

    for item in plan:
        if item['type'] == 'superblock':
            out.write(format_superblock(MAGIC_CONSTANT,
                                        item['first_file_pointer'],
                                        item['first_free_pointer']))
        elif item['type'] == 'free':
            out.write(format_free_block(item['next_pointer'], item['size']))
            if item['size'] > 1:
                out.write('\0' * (SECTOR_SIZE * (item['size'] - 1)))
        elif item['type'] == 'header':
            out.write(format_file_block(item['next_pointer'],
                                        item['file'].name(),
                                        item['file'].size(),
                                        item['is_writeable'],
                                        item['first_chunk_pointer']))
        elif item['type'] == 'chunk_node':
            out.write(format_content_block(item['next_pointer'],
                                           item['chunk_size'],
                                           item['chunk_pointer']))
        elif item['type'] == 'chunk':
            out.write(item['data'])

def format_superblock(magic_constant, first_file_block, first_free_block):
    return pack('=III500s', magic_constant, first_file_block, first_free_block,
                '')

def format_free_block(next_free_block, free_space_size):
    return pack('=II504s', next_free_block, free_space_size, '')

def format_file_block(next_file_block, name, size, is_writeable, content_block):
    return pack('=I256sIII240s', next_file_block, name, size, is_writeable,
                content_block, '')

def format_content_block(next_content_block, size, content):
    return pack('=III500s', next_content_block, size, content, '')

if __name__ == "__main__":
    arg_parser = build_argument_parser()
    options, args = arg_parser.parse_args()
    # Do this before any real work so that we can validate.
    num_sectors = int(options.num_sectors)
    initial_offset = int(options.initial_offset)


    files = []
    for file_string in args:
        name, file_name = file_string.split(':')
        files.append(
            UserFile(name,
                os.path.join(options.search_directory, file_name)))

    plan = build_plan(files, num_sectors)

    f = open(options.output, 'rb+')
    f.seek(initial_offset * SECTOR_SIZE)
    execute_plan(plan, f)
    f.close()
