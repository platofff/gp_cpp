import ctypes
import ctypes.util
import os

# Load shared libraries.
SCRIPT_PATH = os.path.dirname(os.path.abspath(__file__))
if os.name == "nt":
    lib = ctypes.CDLL(os.path.join(SCRIPT_PATH, "bin", "genpattern.dll"))
    libc = ctypes.CDLL(ctypes.util.find_msvcrt())
else:
    lib = ctypes.CDLL(os.path.join(SCRIPT_PATH, "bin", "libgenpattern.so"))
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
    _fields_ = [
        ("x", ctypes.c_ssize_t),
        ("y", ctypes.c_ssize_t)
    ]

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

# Updated function prototype for gp_genpattern (new exception API).
lib.gp_genpattern.argtypes = [
    ctypes.POINTER(C_GPCollection),
    ctypes.c_size_t,  # n_collections
    ctypes.c_size_t,  # canvas_width
    ctypes.c_size_t,  # canvas_height
    ctypes.c_uint8,   # threshold
    ctypes.c_size_t,  # offset_radius
    ctypes.c_size_t,  # collection_offset_radius
    ctypes.POINTER(C_GPSchedule),
    ctypes.c_uint32,  # seed
    ctypes.c_char_p,  # exception_text_buffer
    ctypes.c_size_t   # exception_text_buffer_size
]
lib.gp_genpattern.restype = ctypes.c_int  # Returns 0 on success, non-zero on error.

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
def _extract_offsets(c_coll_ptr: ctypes.c_void_p, n: int) -> list[list[list[tuple[int, int]]]]:
    results: list[list[list[tuple[int, int]]]] = []
    collections = ctypes.cast(c_coll_ptr, ctypes.POINTER(C_GPCollection))
    for i in range(n):
        coll_results: list[list[tuple[int, int]]] = []
        coll = collections[i]
        for j in range(coll.n_images):
            img = coll.images[j]
            offsets: list[tuple[int, int]] = []
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
    collections: list[list[GPImgAlpha]],
    canvas_width: int,
    canvas_height: int,
    threshold: int,
    offset_radius: int,
    collection_offset_radius: int,
    schedule: GPExponentialSchedule | GPLinearSchedule,
    seed: int
) -> list[list[list[tuple[int, int]]]]:
    n_collections = len(collections)
    c_coll_ptr = libc.calloc(n_collections, ctypes.sizeof(C_GPCollection))
    if not c_coll_ptr:
        raise MemoryError("Failed to allocate memory for collections.")
    c_collections = ctypes.cast(c_coll_ptr, ctypes.POINTER(C_GPCollection))
    try:
        for i, coll in enumerate(collections):
            num_images = len(coll)
            c_collections[i].n_images = num_images
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
                src = (ctypes.c_uint8 * data_len).from_buffer_copy(img.data)
                libc.memcpy(data_ptr, ctypes.cast(src, ctypes.c_void_p), data_len)
                c_img.data = ctypes.cast(data_ptr, ctypes.POINTER(ctypes.c_uint8))
                c_img.offsets_size = 0

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

        # Create exception buffer.
        exception_buffer_size = 256
        exception_buffer = ctypes.create_string_buffer(exception_buffer_size)
        ret = lib.gp_genpattern(
            c_collections,
            n_collections,
            canvas_width,
            canvas_height,
            ctypes.c_uint8(threshold),
            ctypes.c_size_t(offset_radius),
            ctypes.c_size_t(collection_offset_radius),
            c_schedule,
            ctypes.c_uint32(seed),
            exception_buffer,
            ctypes.c_size_t(exception_buffer_size)
        )
        libc.free(schedule_ptr)

        if ret != 0:
            err_msg = exception_buffer.value.decode("utf-8")
            raise GPError(err_msg)

        results = _extract_offsets(c_coll_ptr, n_collections)
    finally:
        _free_collections(c_coll_ptr, n_collections)
    return results
