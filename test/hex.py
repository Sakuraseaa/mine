import sys

def main():
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <number>")
        return 1
    
    try:
        number = int(sys.argv[1])
        print(f"{number:x}")
        return 0
    except ValueError:
        print(f"Invalid number: {sys.argv[1]}")
        return 1

if __name__ == "__main__":
    sys.exit(main())
