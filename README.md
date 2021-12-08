# coroutines-epoll-example
OTUS C++ course demo day examples

## Инструкция по сборке
Необходимы следующие версии компонентов
* g++11
* cmake >= 3.10

```
git clone https://github.com/sdukshis/coroutines-epoll-example.git
cmake -B build
cmake --build build
```

Также можно использовать docker образ с подготовленным окружением
```
docker run --rm -ti -p12345:12345 -v"$PWD":/usr/src/app sdukshis/coroutines_epoll_example
```
