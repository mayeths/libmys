import re

# https://stackoverflow.com/a/60708339
def parse_readable_size(size: str):
    # based on https://stackoverflow.com/a/42865957/2002471
    units = {
        "B": 1,
        "KB": 2**10,
        "MB": 2**20,
        "GB": 2**30,
        "TB": 2**40,
        "PB": 2**50,
        "EB": 2**60,
        "ZB": 2**70
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
