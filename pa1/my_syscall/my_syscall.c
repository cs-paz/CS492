#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/printk.h>

SYSCALL_DEFINE1(christian_syscall, char __user *, str) {
	char newStr[32];
	long response = strncpy_from_user(newStr, str, sizeof(newStr));
	int i = 0, replacements = 0;

	if(response < 0 || response > 31){
		return -1;
	}	

	printk(KERN_INFO "before: %s\n", newStr);
	
	while(newStr[i] != '\0') {
		if(newStr[i] == 'a' || newStr[i] == 'e' || newStr[i] == 'i' || newStr[i] == 'o' || newStr[i] == 'u' || newStr[i] == 'y') {
			newStr[i] = 'X';
			replacements++;
		}	
		i++;
	}
	
	printk(KERN_INFO "after: %s\n", newStr);
	copy_to_user(str, newStr, sizeof(newStr));
	return replacements;  
}

