## taskRoutes

### Задача
в текстовом файле организовать хранение данных об автобусных маршрутах. Маршрут содержит названия остановок (можно буквенные или числовые), время движения между соседними остановками, время нахождения на остановке для посадки пассажира. Рассчитать кратчайший маршрут и время движения из точки А в точку Б, вводимых пользователем. Расчет должен производиться в режиме реального времени, при котором считаем положение автобуса на предыдущей остановке в момент запроса (на текущей, если она конечная). Предусмотреть возможность пересадок, если не существует прямого маршрута. В случае пересадки, автобуса пересадочного маршрута находится в середине пути с предыдущей своей остановки.

* Использован язык: C++ (стандарт c++17)
* Для работы с json использована библиотека nlohmann/json
