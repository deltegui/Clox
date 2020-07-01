#include <string.h>
#include "table.h"
#include "memory.h"
#include "object.h"

#define TABLE_MAX_LOAD 0.75

static Entry* find_entry(Entry* entries, int length, ObjString* key);
static void adjust_capacity(Table* table, int capacity);

void init_table(Table* table) {
    table->capacity = 0;
    table->count = 0;
    table->entries = NULL;
}

void free_table(Table* table) {
    FREE_ARRAY(Entry, table->entries, table->capacity);
    init_table(table);
}

bool table_set(Table* table, ObjString* key, Value value) {
    if(table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
        int capacity = GROW_CAPACITY(table->capacity);
        adjust_capacity(table, capacity);
    }
    Entry* entry = find_entry(table->entries, table->capacity, key);
    bool is_new = entry->label == NULL;
    if(is_new && IS_NIL(entry->value)) table->count++;
    entry->label = key;
    entry->value = value;
    return is_new;
}

static Entry* find_entry(Entry* entries, int length, ObjString* key) {
    uint32_t index = key->hash % length;
    Entry* tombstone = NULL;

    for(;;) {
        Entry* entry = &entries[index];
        if(entry->label == NULL) {
            if(IS_NIL(entry->value)) {
                return tombstone != NULL ? tombstone : entry;
            }
            if(tombstone != NULL) tombstone = entry;
        }
        if(entry->label == key) {
            return entry;
        }
        index = (index + 1) % length;
    }
}

static void adjust_capacity(Table* table, int capacity) {
    Entry* entries = ALLOCATE(Entry, capacity);

    for(int i = 0; i < capacity; i++) {
        entries[i].label = NULL;
        entries[i].value = NIL_VALUE();
    }

    table->count = 0;

    for(int i = 0; i < table->capacity; i++) {
        Entry* old_entry = &table->entries[i];
        if(old_entry->label == NULL) continue;
        Entry* position = find_entry(entries, capacity, old_entry->label);
        position->label = old_entry->label;
        position->value = old_entry->value;
        table->count++;
    }

    FREE_ARRAY(Entry, table->entries, table->capacity);
    table->capacity = capacity;
    table->entries = entries;
}

void table_add_all(Table *from, Table* to) {
    for(int i = 0; i < from->capacity; i++) {
        Entry* entry = &from->entries[i];
        if(entry->label != NULL) {
            table_set(to, entry->label, entry->value);
        }
    }
}

bool table_get(Table* table, ObjString* key, Value* value) {
    if(table->capacity == 0) return false;

    Entry* entry = find_entry(table->entries, table->capacity, key);
    if(entry->label == NULL) return false;

    *value = entry->value;
    return true;
}

bool table_delete(Table* table, ObjString* key) {
    if(table->capacity == 0) return false;

    Entry* entry = find_entry(table->entries, table->capacity, key);
    if(entry->label == NULL) return false;

    entry->label = NULL;
    entry->value = BOOL_VALUE(true); // Place tombstone
    return true;
}

ObjString* table_find_string(Table* table, const char* chars, int length, uint32_t hash) {
    if(table->count == 0) return NULL;

    uint32_t index = hash % table->capacity;

    for(;;) {
        Entry* entry = &table->entries[index];
        if(entry->label == NULL) {
            if(IS_NIL(entry->value)) return NULL;
        } else if(entry->label->length == length &&
            entry->label->hash == hash &&
            memcmp(entry->label->chars, chars, length) == 0) {
            return entry->label;
        }
        index = (index + 1) % table->capacity;
    }
}

void mark_table(Table* table) {
    for(int i = 0; i < table->capacity; i++) {
        Entry* entry = &table->entries[i];
        mark_object((Obj*)entry->label);
        mark_value(entry->value);
    }
}
