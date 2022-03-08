touch touched.txt
ln -sf /usr/share/dict/words
head -5 words > first5words.txt
cat /proc/self/status | grep nonvol > nonvoluntaryswitches.txt
sort -R < words | head -5 > rand5words.txt
sort -R < words | head -5 | sort -d > randsort5words.txt
head -5 <(sort -R words) | sort -d > randsort5wordsprocsub.txt