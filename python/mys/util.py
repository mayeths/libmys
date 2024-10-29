import re

def to_readable_size(bytes: int, precision: int = 2) -> str:
    units = ["Bytes", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"]
    size = float(bytes)
    i = 0

    while size >= 1024 and i < len(units) - 1:
        size /= 1024
        i += 1

    return f"{size:.{precision}f} {units[i]}"

# # Example usage
# size_in_bytes = 123456789
# readable_size = to_readable_size(size_in_bytes, precision=2)
# print(readable_size)  # Output: "117.74 MB"

# https://stackoverflow.com/a/60708339
def parse_readable_size(size: str):
    # based on https://stackoverflow.com/a/42865957/2002471
    units = {
        "B": 1,
        "KB": 2**10, "K": 2**10,
        "MB": 2**20, "M": 2**20,
        "GB": 2**30, "G": 2**30,
        "TB": 2**40, "T": 2**40,
        "PB": 2**50, "P": 2**50,
        "EB": 2**60, "E": 2**60,
        "ZB": 2**70, "Z": 2**70,
    }
    try:
        if not re.match(r' ', size):
            size = re.sub(r'([KMGT]?B)(yte(s)?)?', r' \1', size)
        number, unit = [string.strip() for string in size.split()]
        return int(float(number)*units[unit])
    except:
        return -1

# example_strings = [
#     "1024b", "1024B", "1024Byte", "1024Bytes",
#     "10.43 KB", "11 GB", "343.1 MB", "10.43KB",
#     "11GB", "343.1MB", "10.43 kb", "11 gb",
#     "343.1 mb", "10.43kb", "11gb", "343.1mb"
# ]

# for example_string in example_strings:
#         print(example_string, parse_readable_size(example_string))
