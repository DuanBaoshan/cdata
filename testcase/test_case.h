#ifndef _TEST_CASE_H_
#define _TEST_CASE_H_

typedef struct
{
    char *p_name;
    void (*Init)(void);
    void (*Show)(void);
    void (*Run)(int id);
    int  (*GetTestcaseCount)(void);
    void (*Finalize)(void);
}TestcaseSet_t;

typedef int (*TestcaseFn_t)(void);

typedef struct
{
    char *p_description;
    TestcaseFn_t testcaseFn;
}Testcase_t;

typedef struct
{
    char *p_message;
    int   msgLen;
}Message_t;

#endif //_TEST_CASE_H_
