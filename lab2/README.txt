LAB 2 (метод Гаусса, потоки pthread)

СБОРКА:
cmake -S . -B cmake-build-debug && cmake --build cmake-build-debug

ФОРМАТ ЗАПУСКА:
./cmake-build-debug/gaussian <N> [потоки1 потоки2 ...]
Если потоки не указаны, используется 4.

ТЕСТ 1 (маленькая матрица, ожидается: residual порядка 1e-13 или меньше,
        speedup около или меньше 1, потому что N мало):
./cmake-build-debug/gaussian 100 1 2 4

ТЕСТ 2 (большая матрица, ожидается: ускорение растёт с числом потоков,
        до числа ядер процессора):
./cmake-build-debug/gaussian 1000 1 2 4 8

ТЕСТ 3 (много потоков, ожидается: эффективность падает, результат верный):
./cmake-build-debug/gaussian 512 1 2 4 8 16 128

ТЕСТ 4 (ошибки, ожидается: сообщение об ошибке и код возврата 1):
./cmake-build-debug/gaussian
./cmake-build-debug/gaussian abc
./cmake-build-debug/gaussian 100 0
./cmake-build-debug/gaussian 1

ЧИСЛО ЯДЕР (для пояснения результатов):
nproc

ПОКАЗАТЬ ПОТОКИ ВО ВРЕМЯ РАБОТЫ (второй терминал или одна строка):
./cmake-build-debug/gaussian 2000 8 & sleep 1; ps -L -p $! | wc -l; wait