#!/usr/bin/env python3
"""Check docs/coverage-matrix.md against the code.

Fails (exit 1) when a symbol the matrix cites in its API column is not in inc/, src/ or docs/hardware.yml, when a test
id in the Test column is not named in tests/, when a row is malformed, or when the counts in the header are wrong.
A row the driver does not cover must say `GAP` and start its API cell with "not provided".
"""
import collections
import glob
import re
import sys

STATUSES = ("OK", "GUARD", "TODO", "INFO", "GAP")
# Words that appear in backticks but are C++ keywords or names of the standard library or of the test model.
ALLOWED = {"static_assert", "constexpr", "nullptr", "true", "false", "uint8_t", "uint16_t", "uint32_t", "std"}


def strip_comments(text, path):
    """Code only: a symbol that appears in a comment alone is not a symbol."""
    if path.endswith(".yml"):
        return re.sub(r"(?m)^\s*#.*$", "", text)
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    return re.sub(r"//[^\n]*", "", text)


def words(patterns):
    found = set()
    for pattern in patterns:
        for path in glob.glob(pattern):
            with open(path, encoding="utf-8") as handle:
                found |= set(re.findall(r"[A-Za-z_][A-Za-z0-9_]*", strip_comments(handle.read(), path)))
    return found


def main():
    code = words(["inc/*", "src/*", "docs/hardware.yml"])
    test_text = ""
    for path in glob.glob("tests/*.cpp"):
        with open(path, encoding="utf-8") as handle:
            test_text += handle.read()
    errors = []
    counts = collections.Counter()
    ids = set()
    header_total = None
    header_counts = {}
    with open("docs/coverage-matrix.md", encoding="utf-8") as handle:
        lines = handle.read().split("\n")
    for number, line in enumerate(lines, 1):
        found = re.match(r"Total rows (\d+): (.*)\.$", line)
        if found:
            header_total = int(found.group(1))
            header_counts = {m[0]: int(m[1]) for m in re.findall(r"(\w+) (\d+)", found.group(2))}
        if not line.startswith("| ") or line.startswith(("| Id ", "| ---")):
            continue
        cells = [c.strip() for c in line.strip().strip("|").split(" | ")]
        if len(cells) != 6:
            errors.append(f"line {number}: {len(cells)} cells, expected 6")
            continue
        row, _item, _ref, api, test, status = cells
        if row in ids:
            errors.append(f"line {number}: duplicate id {row}")
        ids.add(row)
        if status not in STATUSES:
            errors.append(f"line {number}: {row}: unknown status {status}")
        counts[status] += 1
        if (status == "GAP") != api.startswith("not provided"):
            errors.append(f"line {number}: {row}: a GAP row starts with 'not provided', and only a GAP row does")
        if status != "GAP":
            for token in re.findall(r"`([^`]*)`", api):
                for ident in re.findall(r"[A-Za-z_][A-Za-z0-9_]*", token):
                    if len(ident) > 2 and ident not in code and ident not in ALLOWED:
                        errors.append(f"line {number}: {row}: symbol {ident} is not in the code")
        for test_id in re.findall(r"\bT\d\d\b", test):
            if f'TEST_CASE("{test_id} ' not in test_text:
                errors.append(f"line {number}: {row}: test {test_id} is not in tests/")
    if header_total != sum(counts.values()):
        errors.append(f"header says {header_total} rows, the tables have {sum(counts.values())}")
    for status in STATUSES:
        if header_counts.get(status, 0) != counts[status]:
            errors.append(f"header says {status} {header_counts.get(status, 0)}, the tables have {counts[status]}")
    for error in errors[:60]:
        print("check_matrix:", error)
    if errors:
        print(f"check_matrix: {len(errors)} problem(s)")
        return 1
    print(f"check_matrix: {sum(counts.values())} rows, every cited symbol and test exists")
    return 0


if __name__ == "__main__":
    sys.exit(main())
