
file_path="input.txt"
md5_value=$(md5sum "$file_path" | awk '{print $1}') # 使用md5sum命令计算md5值，并提取结果
echo "before MD5 value of $file_path is: $md5_value" # 输出md5值
dd if=/dev/urandom of=$file_path bs=1M count=1024
md5_value_in=$(md5sum "$file_path" | awk '{print $1}')
echo "after- MD5 value of $file_path is: $md5_value_in"
gcc -o producer-consumer producer-consumer.c -lpthread
./producer-consumer input.txt output.txt
fout_path="output.txt"
md5_value_out=$(md5sum "$fout_path" | awk '{print $1}')
echo "copy-- MD5 value of $fout_path is: $md5_value_out"


