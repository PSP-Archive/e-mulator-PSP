int abs(int a)
{
    //return (c<0) ? -c: c;
    int mask = a >> 31;
    return (a ^ mask) - mask;
}

int max(int a, int b)
{
    int t = (a-b);
    return a - (t & (t >> 31));
}

int min(int a, int b)
{
    int t = (a-b);
    return b + (t & (t >> 31));
}

// copy from pspsdk
int atoi(const char *s)
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



unsigned long atoh(const unsigned char *pszStr)
{
	unsigned long ulVal = 0;
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
