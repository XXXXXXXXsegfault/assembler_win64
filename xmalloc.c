void *xmalloc(long long int size)
{
	void *ptr;
	if(size==0)
	{
		size=1;
	}
	ptr=malloc(size);
	if(ptr==NULL)
	{
		puts("FATAL: cannot allocate memory.");
		puts("Please press any key to continue.");
		getch();
		exit(1);
	}
	return ptr;
}
long long int __str_size(long long int size)
{
	long long int val;
	val=128;
	while(val<size)
	{
		val=val*3>>1;
	}
	return val;
}
char *xstrdup(char *str)
{
	long long int l;
	char *new_str;
	l=strlen(str);
	new_str=xmalloc(__str_size(l+1));
	memcpy(new_str,str,l);
	new_str[l]=0;
	return new_str;
}
