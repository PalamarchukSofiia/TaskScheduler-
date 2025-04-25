
## 🌸 TTaskScheduler —  шедулер мечты 💗

- 💡 **Цель**: создать класс `TTaskScheduler`, который умеет выполнять связанные между собой задачи

- 🌈 Оптимизация: исключаем повторные вычисления, считаем только нужное!

- 👩‍💻 Пример использования (решаем квадратное уравнение без лишних sqrt):

  ```cpp
  struct AddNumber {
    float add(float a) const {
      return a + number;
    }
    float number;
  };

  float a = 1;
  float b = -2;
  float c = 0;
  AddNumber add{ .number = 3 };

  TTaskScheduler scheduler;

  auto id1 = scheduler.add([](float a, float c) { return -4 * a * c; }, a, c);
  auto id2 = scheduler.add([](float b, float v) { return b * b + v; }, b, scheduler.getFutureResult<float>(id1));
  auto id3 = scheduler.add([](float b, float d) { return -b + std::sqrt(d); }, b, scheduler.getFutureResult<float>(id2));
  auto id4 = scheduler.add([](float b, float d) { return -b - std::sqrt(d); }, b, scheduler.getFutureResult<float>(id2));
  auto id5 = scheduler.add([](float a, float v) { return v / (2 * a); }, a, scheduler.getFutureResult<float>(id3));
  auto id6 = scheduler.add([](float a, float v) { return v / (2 * a); }, a, scheduler.getFutureResult<float>(id4));
  auto id7 = scheduler.add(&AddNumber::add, add, scheduler.getFutureResult<float>(id6));

  scheduler.executeAll();

  std::cout << "x1 = " << scheduler.getResult<float>(id5) << std::endl;
  std::cout << "x2 = " << scheduler.getResult<float>(id6) << std::endl;
  std::cout << "x3 = " << scheduler.getResult<float>(id7) << std::endl;

## 🧩 Интерфейс `TTaskScheduler`

- `add(...)` — добавляет задачу, возвращает ID
- `getFutureResult<T>(id)` — возвращает "будущий" результат задачи `id` типа `T`
- `getResult<T>(id)` — возвращает готовый результат задачи `id` (если не посчитан — считает)
- `executeAll()` — запускает все добавленные задачи по зависимостям

---

## 📎 Правила и фишки

- ✅ Поддерживаются любые `Callable`-объекты (в том числе указатели на методы)
- 🧮 Не больше **двух аргументов**
- 🧠 Автоматическое построение зависимостей между задачами
- 🚫 Запрещено использовать стандартную библиотеку, **кроме**:
  - 📦 Контейнеров (`<vector>`, `<map>` и др.)
  - 🧠 Умных указателей (`std::shared_ptr`, `std::unique_ptr`)

---

## 💗 Идеально подходит для

- 🧬 Графов вычислений
- 🌸 Реактивных систем
- 🚀 Параллельных пайплайнов
- 🧠 Интеллектуальной оптимизации

---

## ✨ Вдохновлён мечтой, работает на 💕
