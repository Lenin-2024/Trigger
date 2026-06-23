#include <check.h>
#include <stdlib.h>
#include <string.h>
#include "raylib.h"
#include "../src/engine/engine_context.h"
#include "../src/engine/engine.h"


// Тестовые структуры данных
typedef struct {
    int value;
    int update_called;
    int draw_called;
    int destroy_called;
} testData;

// Тестовые функции для vtable
void testUpdate(void *data, engine_context_t context) {
    testData *test_data = (testData *)data;
    test_data->update_called++;
    test_data->value++;
}

void testDraw(void *data, engine_context_t context) {
    testData *test_data = (testData *)data;
    test_data->draw_called++;
}

void testDestroy(void *data, engine_context_t context) {
    testData *test_data = (testData *)data;
    test_data->destroy_called++;
}

// тестовая vtable
object_v_table_t* createFullVtable(void) {
    static object_v_table_t vtable = {
        .update = testUpdate,
        .draw = testDraw,
        .destroy = testDestroy
    };
    return &vtable;
}

object_v_table_t* createPartialVtable(void) {
    static object_v_table_t vtable = {
        .update = testUpdate,
        .draw = NULL,
        .destroy = testDestroy
    };
    return &vtable;
}

object_v_table_t* createNullVtable(void) {
    static object_v_table_t vtable = {
        .update = NULL,
        .draw = NULL,
        .destroy = NULL
    };
    return &vtable;
}

engine_context_t* createMinimalContext(){
    engine_context_t* context = (engine_context_t*) malloc(sizeof(engine_context_t));
    return context;
} 

// создание entity_manager
START_TEST(test_create_entity_manager) {
    engine_context_t* context = createMinimalContext();
    context->entity_manager = create_entity_manager(10);
    entity_manager_t *manager = context->entity_manager;
    
    ck_assert_ptr_nonnull(manager);
    ck_assert_int_eq(manager->capacity, 10);
    ck_assert_int_eq(manager->count, 0);
    ck_assert_ptr_nonnull(manager->entities);
    
    destroy_entity(manager);
    free(manager->entities);
    free(manager);
    free(context);
}
END_TEST

// capacity = 0
START_TEST(test_create_entity_manager_zero_capacity) {
    engine_context_t* context = createMinimalContext();
    context->entity_manager = create_entity_manager(0);
    entity_manager_t *manager = context->entity_manager;
    
    ck_assert_ptr_nonnull(manager);
    ck_assert_int_eq(manager->capacity, 0);
    ck_assert_int_eq(manager->count, 0);
    
    destroy_entity(manager);
    free(manager->entities);
    free(manager);
    free(context);
}
END_TEST

// создание объекта
START_TEST(test_create_entity) {
    engine_context_t* context = createMinimalContext();
    context->entity_manager = create_entity_manager(10);
    entity_manager_t *manager = context->entity_manager;
    ck_assert_ptr_nonnull(manager);
    
    testData *data = malloc(sizeof(testData));
    data->value = 42;
    data->update_called = 0;
    data->draw_called = 0;
    data->destroy_called = 0;
    
    entity_t *entity = create_entity(manager, data, createFullVtable());
    
    ck_assert_ptr_nonnull(entity);
    ck_assert_int_eq(manager->count, 1);
    ck_assert_int_eq(entity->active, 1);
    ck_assert_ptr_eq(entity->data, data);
    
    destroy_entity(manager);
    free(manager->entities);
    free(manager);
    free(context);
}
END_TEST

// авто-расширение
START_TEST(test_auto_expand_capacity) {
    engine_context_t* context = createMinimalContext();
    context->entity_manager = create_entity_manager(2);
    entity_manager_t *manager = context->entity_manager;
    ck_assert_ptr_nonnull(manager);
    
    // Создаем 3 entity (должно вызвать расширение)
    for (int i = 0; i < 3; i++) {
        testData *data = malloc(sizeof(testData));
        data->value = i;
        data->update_called = 0;
        data->draw_called = 0;
        data->destroy_called = 0;
        
        entity_t *entity = create_entity(manager, data, createFullVtable());
        ck_assert_ptr_nonnull(entity);
    }
    
    ck_assert_int_eq(manager->count, 3);
    ck_assert_int_eq(manager->capacity, 4); // capacity * 2 = 4
    
    destroy_entity(manager);
    free(manager->entities);
    free(manager);
    free(context);
}
END_TEST

// обновление
START_TEST(test_update_all_entities) {
    engine_context_t* context = createMinimalContext();
    context->entity_manager = create_entity_manager(2);
    entity_manager_t *manager = context->entity_manager;
    ck_assert_ptr_nonnull(manager);
    
    testData *data1 = malloc(sizeof(testData));
    testData *data2 = malloc(sizeof(testData));
    data1->value = 10;
    data2->value = 20;
    data1->update_called = 0;
    data2->update_called = 0;
    
    create_entity(manager, data1, createFullVtable());
    create_entity(manager, data2, createFullVtable());
    
    update_all_entities(context);
    
    ck_assert_int_eq(data1->update_called, 1);
    ck_assert_int_eq(data2->update_called, 1);
    ck_assert_int_eq(data1->value, 11);
    ck_assert_int_eq(data2->value, 21);
    
    destroy_entity(manager);
    free(manager->entities);
    free(manager);
    free(context);
}
END_TEST

// отрисовка
START_TEST(test_draw_all_entities) {
    engine_context_t* context = createMinimalContext();
    context->entity_manager = create_entity_manager(2);
    entity_manager_t *manager = context->entity_manager;
    ck_assert_ptr_nonnull(manager);
    
    testData *data1 = malloc(sizeof(testData));
    testData *data2 = malloc(sizeof(testData));
    data1->draw_called = 0;
    data2->draw_called = 0;
    
    create_entity(manager, data1, createFullVtable());
    create_entity(manager, data2, createFullVtable());
    
    draw_all_entities(context);
    
    ck_assert_int_eq(data1->draw_called, 1);
    ck_assert_int_eq(data2->draw_called, 1);
    
    destroy_entity(manager);
    free(manager->entities);
    free(manager);
    free(context);
}
END_TEST

// неактивные объекты
START_TEST(test_inactive_entities) {
    engine_context_t* context = createMinimalContext();
    context->entity_manager = create_entity_manager(2);
    entity_manager_t *manager = context->entity_manager;
    ck_assert_ptr_nonnull(manager);
    
    testData *data1 = malloc(sizeof(testData));
    testData *data2 = malloc(sizeof(testData));
    data1->update_called = 0;
    data1->draw_called = 0;
    data2->update_called = 0;
    data2->draw_called = 0;
    
    entity_t *entity1 = create_entity(manager, data1, createFullVtable());
    entity_t *entity2 = create_entity(manager, data2, createFullVtable());
    
    entity1->active = 0;
    
    update_all_entities(context);
    draw_all_entities(context);
    
    ck_assert_int_eq(data1->update_called, 0);
    ck_assert_int_eq(data1->draw_called, 0);
    
    ck_assert_int_eq(data2->update_called, 1);
    ck_assert_int_eq(data2->draw_called, 1);
    
    destroy_entity(manager);
    free(manager->entities);
    free(manager);
    free(context);
}
END_TEST

// частичный vtable (отсутствует draw)
START_TEST(test_partial_vtable) {
    engine_context_t* context = createMinimalContext();
    context->entity_manager = create_entity_manager(2);
    entity_manager_t *manager = context->entity_manager;
    ck_assert_ptr_nonnull(manager);
    
    testData *data = malloc(sizeof(testData));
    data->update_called = 0;
    data->draw_called = 0;
    data->destroy_called = 0;
    
    create_entity(manager, data, createPartialVtable());
    
    update_all_entities(context); // update должен работать
    draw_all_entities(context);   // draw не должен вызывать ошибку
    
    ck_assert_int_eq(data->update_called, 1);
    ck_assert_int_eq(data->draw_called, 0);
    
    destroy_entity(manager);
    free(manager->entities);
    free(manager);
    free(context);
}
END_TEST

// NULL список методов
START_TEST(test_null_parameters) {
    engine_context_t* context = createMinimalContext();
    context->entity_manager = create_entity_manager(2);
    entity_manager_t *manager = context->entity_manager;
    ck_assert_ptr_nonnull(manager);
    
    testData *data = malloc(sizeof(testData));
    data->destroy_called = 0;
    
    create_entity(manager, data, createNullVtable());

    update_all_entities(context);
    draw_all_entities(context);
    
    destroy_entity(manager);
    
    ck_assert_int_eq(data->destroy_called, 0);
    
    free(manager->entities);
    free(manager);
    free(context);
}
END_TEST

// destroy_entity
START_TEST(test_destroy_entity_calls_destroy) {
    engine_context_t* context = createMinimalContext();
    context->entity_manager = create_entity_manager(2);
    entity_manager_t *manager = context->entity_manager;
    ck_assert_ptr_nonnull(manager);
    
    testData *data = malloc(sizeof(testData));
    data->destroy_called = 0;
    
    create_entity(manager, data, createFullVtable());
    
    destroy_entity(manager);
    
    ck_assert_int_eq(data->destroy_called, 1);
    
    free(manager->entities);
    free(manager);
    free(context);
}
END_TEST


Suite* entity_manager_suite(void) {
    Suite *s = suite_create("Entity Manager");
    
    TCase *tc_em = tcase_create("entity_manager");
    tcase_add_test(tc_em, test_create_entity_manager);
    tcase_add_test(tc_em, test_create_entity_manager_zero_capacity);
    tcase_add_test(tc_em, test_create_entity);
    tcase_add_test(tc_em, test_auto_expand_capacity);
    suite_add_tcase(s, tc_em);
    
    TCase *tc_func = tcase_create("vtable_call");
    tcase_add_test(tc_func, test_update_all_entities);
    tcase_add_test(tc_func, test_draw_all_entities);
    tcase_add_test(tc_func, test_inactive_entities);
    tcase_add_test(tc_func, test_partial_vtable);
    tcase_add_test(tc_func, test_destroy_entity_calls_destroy);
    suite_add_tcase(s, tc_func);
    
    TCase *tc_extreme = tcase_create("extreme");
    tcase_add_test(tc_extreme, test_null_parameters);
    suite_add_tcase(s, tc_extreme);
    
    return s;
}

int main() {
    int number_failed;
    Suite *s = entity_manager_suite();
    SRunner *sr = srunner_create(s);
    
    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
