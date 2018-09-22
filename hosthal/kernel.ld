SECTIONS {

	.text : {
		. = ALIGN(4096);
		host_vm_text_start = .;
		*(.host.vm.text)
		. = ALIGN(4096);
		host_vm_text_end = .;
	}

	.bss (NOLOAD) : {
		. = ALIGN(4096);
		host_vm_bss_start = .;
		*(.host.vm.bss)
		. = ALIGN(4096);
		host_vm_bss_end = .;
	}

}

