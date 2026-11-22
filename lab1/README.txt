LAB 1 (вариант 10)

СБОРКА:
cmake -S . -B cmake-build-debug && cmake --build cmake-build-debug

ЗАПУСК:
./cmake-build-debug/os_labs
(ввести имя файла: lab1/numbers.txt)

ТЕСТ 1 (ожидается: 4 9 15 100):
printf "4\n9\n15\n100\n7\n20\n" > /tmp/a.txt
echo /tmp/a.txt | ./cmake-build-debug/os_labs

ТЕСТ 2 (ожидается: 4 9):
printf "4\n9\n-5\n10\n" > /tmp/b.txt
echo /tmp/b.txt | ./cmake-build-debug/os_labs