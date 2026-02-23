#include <core/asserts.hpp>
#include <core/logger.hpp>
#include <cstdlib>

int main() {
    if (log_init()) {
        // Тестируем логи

        TE_LOG_INFO("Hello from testbed! Число: %d", 42);
        TE_LOG_WARN("Это предупреждение");
        TE_LOG_ERROR("Ошибка с сообщением: %s", "что-то пошло не так");

        // Тестируем ассерты (в Debug они должны останавливаться)
        TE_ASSERT(1 == 1);  // не должно сработать
        TE_ASSERT(1 == 0);  // должно остановить (если Debug)

        // Проверяем рантайм-фильтрацию
        TE_LOG_DEBUG("Это DEBUG-сообщение (до фильтра)");
        te_set_log_level(LogLevel::LL_WARN);
        TE_LOG_DEBUG("Это DEBUG-сообщение (после фильтра) – не должно выводиться");
        TE_LOG_WARN("А это WARN – должно вывестись");

        /*
                TE_LOG_DEBUG("A test message: %f", 3.14f);
                TE_LOG_ERROR("A test message: %f", 3.14f);
                TE_LOG_FATAL("A test message: %f", 3.14f);
                TE_LOG_INFO("A test message: %f", 3.14f);
                TE_LOG_TRACE("A test message: %f", 3.14f);
                TE_LOG_WARN("A test message: %f", 3.14f); */
    }
    system("pause");

    return 0;
}
