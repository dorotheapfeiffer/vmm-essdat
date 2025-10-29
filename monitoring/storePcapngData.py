#!/usr/bin/env python3
import subprocess
import time
import argparse
import sys

def port_type(value: str) -> int:
    try:
        p = int(value)
    except ValueError:
        raise argparse.ArgumentTypeError("Port must be an integer.")
    if not (1 <= p <= 65535):
        raise argparse.ArgumentTypeError("Port must be between 1 and 65535.")
    return p

def main():
    parser = argparse.ArgumentParser(
        description="Automated packet capture loop using dumpcap.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument("-n", "--num-files", type=int, default=1000,
                        help="Number of capture files to create")
    parser.add_argument("-f", "--filename", default="data",
                        help="Base filename for captures")
    parser.add_argument("-i", "--interface", required=True,
                        help="Network interface to capture from (e.g. enp0s31f6)")
    parser.add_argument("-p", "--port", type=port_type, required=True,
                        help="UDP port to capture (BPF capture filter: 'udp port <PORT>')")

    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument("-d", "--duration", type=int,
                       help="Duration per file in seconds")
    group.add_argument("-s", "--filesize", type=int,
                       help="Filesize limit per file in kB")

    args = parser.parse_args()

    # Build BPF capture filter expression
    bpf_filter = f"udp port {args.port}"

    for cnt in range(args.num_files):
        timestamp = time.strftime("%Y%m%d%H%M%S")
        filename = f"{args.filename}_{cnt:05}_{timestamp}.pcapng"

        # Build dumpcap arguments
        if args.duration:
            capture_limit = f"duration:{args.duration}"
        else:
            capture_limit = f"filesize:{args.filesize}"

        cmd = [
            "dumpcap",
            "-w", filename,
            "-a", capture_limit,
            "-i", args.interface,
            "-f", bpf_filter,         # <-- apply the UDP port filter
        ]

        print(f"[{cnt+1}/{args.num_files}] Capturing to {filename} with filter '{bpf_filter}' ...")
        try:
            subprocess.call(cmd)
        except KeyboardInterrupt:
            print("\nCapture interrupted by user.")
            sys.exit(0)
        except OSError as e:
            print(f"Error running dumpcap: {e}")
            sys.exit(1)

    print(f"\n✅ Finished capturing {args.num_files} files.")

if __name__ == "__main__":
    main()
