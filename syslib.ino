int htoi(char c) {
  if('0' <= c && c <= '9') {
    return (c - 0x30);
  } else if('A' <= c && c <= 'F') {
    return (c - 0x37); // (c - 0x41) + 0x0A = 0x37
  } else {
    return -1;
  }
}

char itoh(int n) {
  if(0 <= n && n <= 9) {
    return (n + 0x30);
  } else if(10 <= n && n <= 15) {
    return (n + 0x37); // (n - 0x0A) + 0x41 = 0x37
  } else {
    return '0';
  }
}

