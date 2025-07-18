# Fat32


I've been working on a low-level C programming project that dives deep into the FAT32 filesystem. Inspired by traditional UNIX commands, I've created a suite of tools that replicate core functionalities — but for FAT32!
Tools implemented:
mountFat32: Mount and parse the FAT32 filesystem (including the BPB - BIOS Parameter Block)
📄 lsdir: List directory contents (like ls)
📁 cdir: Change directory (like cd)
📍 pwdir: Show current working directory (like pwd)
🧾 catfile: Read and display file content (like cat)
🧾 readFat32Bpb: Read the FAT32 BPB info from a device.

✨ Along the way, I’ve learned to:
Parse BPB fields such as sector size, FAT tables, root cluster, and FSInfo.
Traverse FAT chains to reconstruct files.
Handle cluster allocation visualizations.
Simulate a mini shell interface for file interaction.

It’s been a rewarding experience combining systems programming, reverse engineering, and a passion for understanding how filesystems work under the hood.
If you're exploring embedded systems, filesystems, or C programming, let’s connect — I’d love to exchange insights!

