LAB 4 (вариант 10: cos_derivative и gcd, динамические библиотеки)

СБОРКА (из корня репозитория ~/labs/os-labs):
cmake -S . -B cmake-build-debug && cmake --build cmake-build-debug

После сборки в cmake-build-debug должны быть:
libimpl1.so, libimpl2.so, lab4_prog1, lab4_prog2
Проверка:
ls cmake-build-debug | grep -E "libimpl|lab4"

РАЗОБРАТЬ (удалить сборку):
rm -rf cmake-build-debug

ЗАПУСК (выход: Ctrl+D):
./cmake-build-debug/lab4_prog1    # библиотека подключена при линковке
./cmake-build-debug/lab4_prog2    # библиотеки через dlopen, команда 0 переключает реализацию

КОМАНДЫ ПРОГРАММ:
0              переключить реализацию (только prog2)
1 a dx         производная cos в точке a с приращением dx
2 a b          НОД двух натуральных чисел

ТЕСТ 1: программа 1
Ожидается: производная около -0.8417, НОД(48, 18) = 6
printf "1 1 0.001\n2 48 18\n" | ./cmake-build-debug/lab4_prog1

ТЕСТ 2: программа 2 с переключением
Ожидается: реализация 1 даёт около -0.8417, после команды 0 реализация 2 даёт около -0.8415, НОД = 6
printf "1 1 0.001\n0\n1 1 0.001\n2 48 18\n" | ./cmake-build-debug/lab4_prog2

ТЕСТ 3: ошибки ввода
Ожидается: сообщения "Ошибка: ..." на каждую строку
printf "1 1 0\n2 0 5\n7\nabc\n" | ./cmake-build-debug/lab4_prog2

ТЕСТ 4: разница подключения библиотек
Ожидается: у prog1 строка с libimpl1.so, у prog2 пусто
ldd cmake-build-debug/lab4_prog1 | grep libimpl
ldd cmake-build-debug/lab4_prog2 | grep libimpl || echo "libimpl не найдена: прямой зависимости нет"

ТЕСТ 5: prog2 загружает библиотеки в рантайме (нужен strace: sudo apt install strace)
Ожидается: два вызова openat для libimpl1.so и libimpl2.so
printf "0\n" | strace -e trace=openat ./cmake-build-debug/lab4_prog2 2>&1 | grep libimpl

ТЕСТ 6 (по желанию): скорость НОД, Евклид и перебор
Ожидается: первая команда мгновенно, вторая заметно дольше
time (echo "2 2000000000 1999999999" | ./cmake-build-debug/lab4_prog1)
time (printf "0\n2 2000000000 1999999999\n" | ./cmake-build-debug/lab4_prog2)