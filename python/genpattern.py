import ctypes.util
from cffi import FFI
import re
import os



ffi = FFI()

with open('genpattern.h', 'r') as hf:
    ffi.cdef(re.search(r'\/\/ CFFI_BEGIN.*?\/\/ CFFI_END', hf.read(), re.DOTALL).group().replace('GP_API', ''))

if os.name == 'nt':
    lib = ffi.dlopen('genpattern.dll')
    libc = ffi.dlopen(ctypes.util.find_library('msvcrt'))
else:
    lib = ffi.dlopen('./libgenpattern.so')
    libc = ffi.dlopen(ctypes.util.find_library('c'))

ffi.cdef('''void *calloc( size_t num, size_t size );
void* memcpy( void *restrict dest, const void *restrict src, size_t count );
void free( void *ptr );''')


class GPImgAlpha:
    def __init__(self, width, height, data):
        self.data = data
        self.width = width
        self.height = height

class GPSchedule:
    pass

class GPExponentialSchedule:
    def __init__(self, alpha):
        self.alpha = alpha

class GPLinearSchedule:
    def __init__(self, k):
        self.k = k

class GPError(BaseException): pass

def extract_offsets(c_collections, size):
    extracted_offsets = []

    for i in range(size):
        collection = c_collections[i]
        collection_offsets = []

        for j in range(collection.n_images):
            image = collection.images[j]
            image_offsets = [ 
                (image.offsets[k].x, image.offsets[k].y) for k in range(image.offsets_size) 
            ]

            collection_offsets.append(image_offsets)

        extracted_offsets.append(collection_offsets)

    return extracted_offsets


def free_collections(c_collections, size):
    for i in range(size):
        collection = c_collections[i]
        for j in range(collection.n_images):
            image = collection.images[j]
            libc.free(image.data)
        libc.free(collection.images)
    libc.free(c_collections)

def gp_genpattern(collections, canvas_width, canvas_height, threshold, offset_radius, collection_offset_radius, schedule, seed):
    c_collections = ffi.cast('GPCollection*', libc.calloc(len(collections), ffi.sizeof('GPCollection')))

    for i, coll in enumerate(collections):
        c_coll = c_collections[i]
        c_coll.n_images = len(coll)
        c_coll.images = ffi.cast('GPImgAlpha*', libc.calloc(len(coll), ffi.sizeof('GPImgAlpha')))

        for j, img in enumerate(coll):
            c_img = c_coll.images[j]
            c_img.width = img.width
            c_img.height = img.height
            buf = ffi.from_buffer(img.data)
            c_img.data = ffi.cast('uint8_t*', libc.calloc(len(img.data), 1))
            libc.memcpy(c_img.data, buf, len(img.data))

    c_schedule = ffi.cast('GPSchedule*', libc.calloc(1, ffi.sizeof('GPSchedule')))
    if type(schedule) is GPExponentialSchedule:
        c_schedule.type = lib.GP_EXPONENTIAL
        c_schedule.params.exponential.alpha = schedule.alpha
    elif type(schedule) is GPLinearSchedule:
        c_schedule.type = lib.GP_LINEAR
        c_schedule.params.linear.k = schedule.k
    else:
        raise GPError('Unsupported cooling schedule')

    exception = lib.gp_genpattern(
        c_collections,
        len(collections),
        canvas_width,
        canvas_height,
        threshold,
        offset_radius,
        collection_offset_radius,
        c_schedule,
        seed
    )

    libc.free(c_schedule)

    if exception != ffi.NULL:
        text = ffi.string(exception).decode()
        libc.free(exception)
        free_collections(c_collections, len(collections))
        raise GPError(text)

    result = extract_offsets(c_collections, len(collections))
    free_collections(c_collections, len(collections))

    return result