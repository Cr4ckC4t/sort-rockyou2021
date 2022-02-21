# Sorting RockYou2021 Challenge

[RockYou2021.txt](https://github.com/ohmybahgosh/RockYou2021.txt) is a giant wordlist that comes in at about 91GB of passwords. From the original post:

> All passwords are 6-20 characters long, all lines with non-ASCII characters or white space or tab are removed, resulting in 82 billion unique entries.

Now, processing such a huge file quickly becomes unmanageable on casual computers so the idea is to break it down a little bit.

## The Challenge

More out of fun than necessity, the following challenge arose:

**Split the wordlist into smaller line-length based wordlists as fast as possible.**

This can become useful in situations where a password length is known and one does not have the time to run the entire wordlists but rather wants to focus on a wordlist of specific password-lengths.

Simple solutions such as `grep` and a bash `for` loop are not efficient enough and can cause the host to crash when attempting to load the entire content into RAM.

**The basic ideas:**
- divide & conquer
- minimize file IO 
- utilize streams / process the data in chunks (another idea would be to use memory mapped files)

## Usage

In order to utilize very easy multi-threading later on I've split the 91GB into roughly 256MB parts.

```bash
split rockyou2021.txt -C 268435456 --additional-suffix=.part
```
That operation alone took about 10 minutes on my computer but is considered preprocessing for now.

The C code will take one text file as argument and process it in chunks à 4096 Bytes, writing each line to the corresponding output file depending on the length. 

To compile and execute the program:

```bash
gcc -O3 sort-rockyou.c 
for i in $(ls x{a..o}{a..z}.part 2>/dev/null); do time sh -c "./a.out $i" &
```

**Note:** The code can be optimized in a lot of ways but sets a foundation in the processing principles of this huge wordlist.

## Results

The performance test was generously sponsored by a friend of mine with the following setup:
```
2990WX Threadripper
32Cores 64Threads
PCIe_Gen3 Bus on a Seagate Firecuda 510 (1TB) max sequential 3450MB write and 3100MB read/sec
128GB of ram running at 3133Mhz
```
---
Processing all RockYou2021 parts took a total of `28.431` seconds.

The average across multiple tests continued to be 28 seconds.

---
For comparison, processing the *old* `rockyou.txt` (~130MB) (minor code adaptions necessary - need to set the MAX_LINE_LENGTH to ~290!) took 1 second:
```
time sh -c './a.out rockyou.txt'  0.90s user 0.11s system 99% cpu 1.008 total
```
---

**Note:** Using a more casual system the execution time lies somewhere between 5 and 15 minutes generally. More RAM does *not* help (the program isn't RAM intensive currently) while more CPU power and a faster disk medium will help increase performance.

## Caveat

While the RockYou2021 does not contain any non-UTF8 characters applying the code on other wordlists such as the `rockyou.txt` may cause unexpected results, as a string with non-UTF8 characters will land in a wordlist of larger lengths (the total amount of bytes it takes to represent that string).
