#ifndef FUNCTIONS_H
#define FUNCTIONS_H

double round2(double value);
const char* showDuration();
int lastSunday(int month, int year);
bool isDST();
bool isNtpTimeValid(unsigned long epoch);
bool safeNtpUpdate(unsigned long timeoutMs = 200);

#endif
