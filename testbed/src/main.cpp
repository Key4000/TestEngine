#include <core/asserts.hpp>
#include <core/logger.hpp>
#include <cstdlib>

int main() {
    // Тестируем логи
    TE_LOG_INFO("Hello from testbed! Число: %d", 42);
    TE_LOG_WARN("Это предупреждение");
    TE_LOG_ERROR("Ошибка с сообщением: %s", "что-то пошло не так");

    // Тестируем ассерты (в Debug они должны останавливаться)
    TE_ASSERT(1 == 1); // не должно сработать
    TE_ASSERT(1 == 0); // должно остановить (если Debug)

    // Проверяем рантайм-фильтрацию
    TE_LOG_DEBUG("Это DEBUG-сообщение (до фильтра)");
    te_set_log_level(LogLevel::WARN);
    TE_LOG_DEBUG("Это DEBUG-сообщение (после фильтра) – не должно выводиться");
    TE_LOG_WARN("А это WARN – должно вывестись");

    system("pause");

    return 0;
}
