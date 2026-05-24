About My Research
My background in system security began in the 1990s, an era when exploring Windows kernels (3.1, 95, 98, NT 3.51/4.0) meant working "close to the metal" in Assembly. During that period, my work focused on 0-day research and the development of rootkit techniques, providing me with an intimate understanding of kernel structure vulnerabilities.

After a long hiatus from the field in 2003, my return in 2026 was driven by a stark technical observation: despite decades of patches, the Linux kernel remains structurally vulnerable. The relentless accumulation of Ring 0 vulnerabilities—from Dirty COW to contemporary exploits—demonstrates that traditional kernel perimeter defense is insufficient.

My working hypothesis is the following: Ring 0 must be considered a perpetually compromised environment. My current research focuses on securing the system from "below" the kernel, leveraging virtualization mechanisms (VMX) and hardware isolation (EPT) to guarantee operational integrity.

Ring -1 (Hypervisor): The primary vantage point for system introspection and isolation.

Ring -3 (Hardware Backdoors & Silicon-level Persistence): My threat model explicitly accounts for the existence of immutable hardware backdoors, malicious firmware, and compromised platform components.

Note on Research Scope: While my primary technical focus remains on VMX/EPT implementation, I acknowledge that the most resilient system must operate under the assumption that the underlying silicon itself may be fundamentally tainted. Consequently, my work explores the creation of security boundaries that remain effective even against state-level persistence at the hardware/firmware interface.

This repository documents my research, my experiments with x86_64 architecture, and my pursuit of robust system isolation in the face of an ever-present attacker.