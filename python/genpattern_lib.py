import ctypes
import ctypes.util
import os
from typing import List, Tuple, Union

# Load shared libraries.
if os.name == "nt":
    lib = ctypes.CDLL("genpattern.dll")
    libc = ctypes.CDLL(ctypes.util.find_library("msvcrt"))
else:
    lib = ctypes.CDLL("./libgenpattern.so")
    libc = ctypes.CDLL(ctypes.util.find_library("c"))

# Set libc function prototypes.
libc.calloc.restype = ctypes.c_void_p
libc.calloc.argtypes = [ctypes.c_size_t, ctypes.c_size_t]
libc.memcpy.restype = ctypes.c_void_p
libc.memcpy.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_size_t]
libc.free.restype = None
libc.free.argtypes = [ctypes.c_void_p]

# Define C types corresponding to genpattern.h structures.
class C_GPPoint(ctypes.Structure):
    # for some reason there is no standard ptrdiff_t in ctypes :/
    _fields_ = [
        ("x", ctypes.c_ssize_t),
        ("y", ctypes.c_ssize_t)
    ]

# GPVector is identical to GPPoint.
C_GPVector = C_GPPoint

class C_GPImgAlpha(ctypes.Structure):
    _fields_ = [
        ("width", ctypes.c_size_t),
        ("height", ctypes.c_size_t),
        ("data", ctypes.POINTER(ctypes.c_uint8)),
        ("offsets", C_GPPoint * 4),
        ("offsets_size", ctypes.c_uint8)
    ]

class C_GPCollection(ctypes.Structure):
    _fields_ = [
        ("n_images", ctypes.c_size_t),
        ("images", ctypes.POINTER(C_GPImgAlpha))
    ]

# Enum for schedule type.
GP_EXPONENTIAL: int = 0
GP_LINEAR: int = 1

class C_GPExponentialScheduleParams(ctypes.Structure):
    _fields_ = [("alpha", ctypes.c_double)]

class C_GPLinearScheduleParams(ctypes.Structure):
    _fields_ = [("k", ctypes.c_double)]

class C_ScheduleParams(ctypes.Union):
    _fields_ = [
        ("exponential", C_GPExponentialScheduleParams),
        ("linear", C_GPLinearScheduleParams)
    ]

class C_GPSchedule(ctypes.Structure):
    _fields_ = [
        ("type", ctypes.c_uint8),
        ("params", C_ScheduleParams)
    ]

# Set function prototype for gp_genpattern.
lib.gp_genpattern.argtypes = [
    ctypes.POINTER(C_GPCollection),
    ctypes.c_size_t,  # n_collections
    ctypes.c_size_t,  # canvas_width
    ctypes.c_size_t,  # canvas_height
    ctypes.c_uint8,   # threshold
    ctypes.c_size_t,  # offset_radius
    ctypes.c_size_t,  # collection_offset_radius
    ctypes.POINTER(C_GPSchedule),
    ctypes.c_uint32   # seed
]
lib.gp_genpattern.restype = ctypes.c_char_p

# Python-level classes.
class GPError(Exception):
    pass

class GPImgAlpha:
    def __init__(self, width: int, height: int, data: bytes) -> None:
        self.width: int = width
        self.height: int = height
        self.data: bytes = data

class GPExponentialSchedule:
    def __init__(self, alpha: float) -> None:
        self.alpha: float = alpha

class GPLinearSchedule:
    def __init__(self, k: float) -> None:
        self.k: float = k

# Internal helper functions.
def _extract_offsets(c_coll_ptr: ctypes.c_void_p, n: int) -> List[List[List[Tuple[int, int]]]]:
    results: List[List[List[Tuple[int, int]]]] = []
    collections = ctypes.cast(c_coll_ptr, ctypes.POINTER(C_GPCollection))
    for i in range(n):
        coll_results: List[List[Tuple[int, int]]] = []
        coll = collections[i]
        for j in range(coll.n_images):
            img = coll.images[j]
            offsets: List[Tuple[int, int]] = []
            for k in range(img.offsets_size):
                pt = img.offsets[k]
                offsets.append((pt.x, pt.y))
            coll_results.append(offsets)
        results.append(coll_results)
    return results

def _free_collections(c_coll_ptr: ctypes.c_void_p, n: int) -> None:
    if not c_coll_ptr:
        return
    collections = ctypes.cast(c_coll_ptr, ctypes.POINTER(C_GPCollection))
    for i in range(n):
        coll = collections[i]
        if coll.images:
            for j in range(coll.n_images):
                img = coll.images[j]
                if img.data:
                    libc.free(img.data)
            libc.free(ctypes.cast(coll.images, ctypes.c_void_p))
    libc.free(c_coll_ptr)

# Main binding function.
def gp_genpattern(
    collections: List[List[GPImgAlpha]],
    canvas_width: int,
    canvas_height: int,
    threshold: int,
    offset_radius: int,
    collection_offset_radius: int,
    schedule: Union[GPExponentialSchedule, GPLinearSchedule],
    seed: int
) -> List[List[List[Tuple[int, int]]]]:
    n_collections = len(collections)
    # Allocate memory for collections array.
    coll_array_size = n_collections * ctypes.sizeof(C_GPCollection)
    c_coll_ptr = libc.calloc(n_collections, ctypes.sizeof(C_GPCollection))
    if not c_coll_ptr:
        raise MemoryError("Failed to allocate memory for collections.")
    c_collections = ctypes.cast(c_coll_ptr, ctypes.POINTER(C_GPCollection))

    try:
        # Populate each collection.
        for i, coll in enumerate(collections):
            num_images = len(coll)
            c_collections[i].n_images = num_images
            img_array_size = num_images * ctypes.sizeof(C_GPImgAlpha)
            images_ptr = libc.calloc(num_images, ctypes.sizeof(C_GPImgAlpha))
            if not images_ptr:
                raise MemoryError("Failed to allocate memory for images.")
            c_collections[i].images = ctypes.cast(images_ptr, ctypes.POINTER(C_GPImgAlpha))
            for j, img in enumerate(coll):
                c_img = c_collections[i].images[j]
                c_img.width = img.width
                c_img.height = img.height
                data_len = len(img.data)
                data_ptr = libc.calloc(data_len, 1)
                if not data_ptr:
                    raise MemoryError("Failed to allocate memory for image data.")
                # Copy image data.
                src = (ctypes.c_uint8 * data_len).from_buffer_copy(img.data)
                libc.memcpy(data_ptr, ctypes.byref(src), data_len)
                c_img.data = ctypes.cast(data_ptr, ctypes.POINTER(ctypes.c_uint8))
                # offsets and offsets_size will be filled by gp_genpattern.
                c_img.offsets_size = 0

        # Allocate and populate schedule.
        schedule_ptr = libc.calloc(1, ctypes.sizeof(C_GPSchedule))
        if not schedule_ptr:
            raise MemoryError("Failed to allocate memory for schedule.")
        c_schedule = ctypes.cast(schedule_ptr, ctypes.POINTER(C_GPSchedule))
        if isinstance(schedule, GPExponentialSchedule):
            c_schedule.contents.type = GP_EXPONENTIAL
            c_schedule.contents.params.exponential.alpha = schedule.alpha
        elif isinstance(schedule, GPLinearSchedule):
            c_schedule.contents.type = GP_LINEAR
            c_schedule.contents.params.linear.k = schedule.k
        else:
            raise GPError("Unsupported cooling schedule.")

        # Call the C function.
        err_ptr = lib.gp_genpattern(
            c_collections,
            n_collections,
            canvas_width,
            canvas_height,
            ctypes.c_uint8(threshold),
            ctypes.c_size_t(offset_radius),
            ctypes.c_size_t(collection_offset_radius),
            c_schedule,
            ctypes.c_uint32(seed)
        )
        # Free schedule memory.
        libc.free(schedule_ptr)

        if err_ptr:
            err_msg = ctypes.string_at(err_ptr).decode("utf-8")
            libc.free(err_ptr)
            raise GPError(err_msg)

        # Extract results.
        results = _extract_offsets(c_coll_ptr, n_collections)
    finally:
        _free_collections(c_coll_ptr, n_collections)

    return results
