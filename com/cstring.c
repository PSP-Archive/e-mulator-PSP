// 
// core string
// 

//------------------------------------------------------------------------------
// core_memXXXX
//------------------------------------------------------------------------------
int core_memcmp(const void *buf1, const void *buf2,int n)
{
	int ret;
	int i;
	
	for(i=0; i<n; i++){
		ret = ((unsigned char*)buf1)[i] - ((unsigned char*)buf2)[i];
		if(ret!=0)
			return ret;
	}
	return 0;
}

void* core_memcpy(void *buf1, const void *buf2, int n)
{
#if 1
	int i;
	for(i=0;i<n;i++){
		((unsigned char*)buf1)[i] = ((unsigned char*)buf2)[i];
	}
#else
	while(n-->0)
		((unsigned char*)buf1)[n] = ((unsigned char*)buf2)[n];
#endif
	return buf1;
}

void* core_memmove( void* dest, const void* src, int n )
{
	unsigned long int dstp = (long int) dest;
	unsigned long int srcp = (long int) src;
	int		i;

	if (n == 0) return dest;
    
    if ( dstp - srcp >= n ){
        for ( i = 0; i < n; i++ ){
            ((char*)dstp)[i] = ((char*)srcp)[i];
        }
    } else {
        for ( i = 0; i < n; i++ ){
            ((char*)dstp)[n - i - 1] = ((char*)srcp)[n - i - 1];
        }
	}
	return dest;
}


void* core_memset(void *buf, int ch, int n)
{
	unsigned char *p = buf;
	
	while(n>0)
		p[--n] = ch;
	
	return buf;
}

void core_memcpyX4(u32* dst,u32* src,int size)
{
    size/=4;
    
    while(size--){ 
        *dst++ = *src++;
    }
}


void core_memsetX4(u32* dst,u32 clr,int size)
{
    size/=4;
    
    while(size--) {
        *dst++=clr;
    }
}


//------------------------------------------------------------------------------
// core_strXXXX
//------------------------------------------------------------------------------
int core_strlen(const char *s)
{
	int ret;
	
	for(ret=0; s[ret]; ret++)
		;
	
	return ret;
}

char* core_strcpy(char *dest, const char *src)
{
	int i;
	
	for(i=0; src[i]; i++)
		dest[i] = src[i];
	dest[i] = 0;
	
	return dest;
}

char *core_strchr(const char *s, int c)
{
    int		size;
    int		i;
    
    size = core_strlen( s );
    for ( i = 0; i < size; i++ ){
        if ( s[i] == c ){
            return (char*)&s[i];
        }
    }
    return 0;
}

char* core_strrchr(const char *src, int c)
{
	int len;
	
	len=core_strlen(src);
	while(len>0){
		len--;
		if(*(src+len) == c)
			return (char*)(src+len);
	}
	
	return NULL;
}

char* core_strcat(char *dest, const char *src)
{
	int i;
	int len;
	
	len=core_strlen(dest);
	for(i=0; src[i]; i++)
		dest[len+i] = src[i];
	dest[len+i] = 0;
	
	return dest;
}

int core_strcmp(const char *str1, const char *str2)
{
	char c1, c2;
	for(;;){
		c1 = *str1;
		c2 = *str2;
		
		if(c1!=c2)
			return 1;
		else if(c1==0)
			return 0;
		
		str1++; str2++;
	}
}

int core_strncmp(const char *s1, const char* s2, int n)
{
	unsigned char c1 = '\0';
	unsigned char c2 = '\0';

	while (n > 0){
		c1 = (unsigned char) *s1++;
		c2 = (unsigned char) *s2++;
		if (c1 == '\0' || c1 != c2)
			return c1 - c2;
		n--;
	}

	return c1 - c2;
}


int core_stricmp(const char *str1, const char *str2)
{
	char c1, c2;
	for(;;){
		c1 = *str1;
		if(c1>=0x61 && c1<=0x7A) c1-=0x20;
		c2 = *str2;
		if(c2>=0x61 && c2<=0x7A) c2-=0x20;
		
		if(c1!=c2)
			return 1;
		else if(c1==0)
			return 0;
		
		str1++; str2++;
	}
}

void core_strrev(char *s){
	char tmp;
	int i;
	int len = core_strlen(s);
	
	for(i=0; i<len/2; i++){
		tmp = s[i];
		s[i] = s[len-1-i];
		s[len-1-i] = tmp;
	}
}

void core_itoa(int val, char *s) {
	char *t;
	int mod;

	if(val < 0) {
		*s++ = '-';
		val = -val;
	}
	t = s;

	while(val) {
		mod = val % 10;
		*t++ = (char)mod + '0';
		val /= 10;
	}

	if(s == t)
		*t++ = '0';

	*t = '\0';

	core_strrev(s);
}

void core_ustoa(unsigned short val, char *s) {
	char *t;
	unsigned short mod;
	
	t = s;
	
	while(val) {
		mod = val % 10;
		*t++ = (char)mod + '0';
		val /= 10;
	}

	if(s == t)
		*t++ = '0';

	*t = '\0';

	core_strrev(s);
}

void core_ultoa(unsigned long val, char *s) {
	char *t;
	unsigned long mod;
	
	t = s;
	
	while(val) {
		mod = val % 10;
		*t++ = (char)mod + '0';
		val /= 10;
	}

	if(s == t)
		*t++ = '0';

	*t = '\0';

	core_strrev(s);
}

#if 1 
int core_atoi(const unsigned char *pszStr)
{
	unsigned long ulVal = 0;
	int cbI = 0;

	while (pszStr[cbI]) {
		if (pszStr[cbI] >= '0' && pszStr[cbI] <= '9') {
			ulVal *= 10;
			ulVal += pszStr[cbI] - '0';
		}
		else {
			break;
		}
		cbI++;
	}
	return ulVal;
}
#else // ”÷–­‚É‚±‚ÌŠÖ”‚Ì‚¹‚¢‚Ånes_rom‚ª“®‚©‚È‚¢‚Ý‚½‚¢
// copy from pspsdk
int core_atoi(const char *s)
{
  int neg = 0, ret = 1;

  for (;ret;++s) {
    switch(*s) {
      case ' ':
      case '\t':
        continue;
      case '-':
        ++neg;
      case '+':
        ++s;
      default:
        ret = 0;
        break;
    }
  }
  /* calculate the integer value. */
  for (; ((*s >= '0') && (*s <= '9')); ) ret = ((ret * 10) + (int)(*s++ - '0'));
  return ((neg == 0) ? ret : -ret);
}
#endif


unsigned int core_atoh(const unsigned char *pszStr)
{
	unsigned int ulVal = 0;
	int cbI = 0;

	while (pszStr[cbI]) {
		if (pszStr[cbI] >= '0' && pszStr[cbI] <= '9') {
			ulVal <<= 4;
			ulVal += pszStr[cbI] - '0';
		}
		else if (pszStr[cbI] >= 'a' && pszStr[cbI] <= 'f') {
			ulVal <<= 4;
			ulVal += pszStr[cbI] - 'a' + 10;
		}
		else if (pszStr[cbI] >= 'A' && pszStr[cbI] <= 'F') {
			ulVal <<= 4;
			ulVal += pszStr[cbI] - 'a' + 10;
		}
		else {
			break;
		}
		cbI++;
	}
	return ulVal;
}





