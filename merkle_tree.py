from hashlib import blake2b
import sys

data = [line.strip() for line in open(sys.argv[1], 'r').readlines()]

hashes = [blake2b(line.encode()) for line in data]

while len(hashes) >= 2:
    first = hashes.pop(0)
    second = hashes.pop(0)
    hashes.insert(0, blake2b(first.digest()+second.digest()))

print(hashes[0].hexdigest())
