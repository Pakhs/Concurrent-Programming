#include "mypipe.h"

struct _pipe_ **pipeArray = NULL;
int _pipeCount_ = 0;

void pipeInit(struct _pipe_ *pipe) {
    pipe->itmCount = 0;
    pipe->turn = 0;
    pipe->wantR = 0;
    pipe->wantW = 0;
    pipe->r = pipe->buffer;
    pipe->w = pipe->buffer;
    pipe->writeDone = 0;
	pipe->pipeSize = PIPE_SIZE;
    return;
}

int pipe_open(int size) {
    int pos, i;
    
    if (! pipeArray) {
        pos = 0;
        pipeArray = (struct _pipe_ **) malloc(sizeof(struct _pipe_ *));
        _pipeCount_++;
    }
    else {
        for (i = 0; i < _pipeCount_ && pipeArray[i] != NULL; i++);
        if (i == _pipeCount_) {
            pipeArray = (struct _pipe_ **) reallocarray(pipeArray, sizeof(struct _pipe_ *), ++_pipeCount_);
            
            pos = _pipeCount_;
        }
    }
    pipeArray[pos] = (struct _pipe_ *) malloc(sizeof(struct _pipe_));
    pipeArray[pos]->buffer = (char *) malloc(sizeof(char) * size);
    pipeInit(pipeArray[pos]);
    
    return pos;
}

int pipe_writeDone(int p) {
    if (!pipeArray[p]) return -1;
    
    pipeArray[p]->w = NULL;
    pipeArray[p]->writeDone = 1;
    
    return 1;
}

int pipe_read(int p, char *c) {
    if (!pipeArray[p]) return -1;
    
    while (!pipeArray[p]->itmCount && ! pipeArray[p]->writeDone);
    /* gia na bgei 8a prepei h to writeDone == 1 h to itmCount != 0 */
    
    if (pipeArray[p]->writeDone && ! pipeArray[p]-> itmCount) {
        pipeArray[p]->r = NULL;
        
        free(pipeArray[p]->buffer);
        pipeArray[p]->buffer = NULL;
        free(pipeArray[p]);
        pipeArray[p] = NULL;
        
        if (! --_pipeCount_) {
            free(pipeArray);
            pipeArray = NULL;
        }
        return 0;
    }
    
    pipeArray[p]->wantR = 1;
    
    while (pipeArray[p]->wantW) {
        if (!pipeArray[p]->turn) {
            pipeArray[p]->wantR = 0;
            while (!pipeArray[p]->turn && !pipeArray[p]->writeDone);
            pipeArray[p]->wantR = 1;
        }
    }
    
    if (pipeArray[p]->r == pipeArray[p]->buffer + pipeArray[p]->pipeSize)
        pipeArray[p]->r = pipeArray[p]->buffer;
    
    strncpy(c, pipeArray[p]->r, 1);
    (pipeArray[p]->r)++;
    pipeArray[p]->itmCount--;
    
    pipeArray[p]->turn = 0;
    pipeArray[p]->wantR = 0;
    
    return 1;
}

int pipe_write(int p, char c) {
    if (!pipeArray[p]) return -1;
    
    while (pipeArray[p]->itmCount >= pipeArray[p]->pipeSize);
    
    pipeArray[p]->wantW = 1;
    
    while (pipeArray[p]->wantR) {
        if (pipeArray[p]->turn) { // an einai h seira tou allou, paraxwrhse proteraiothta, perimene na er8ei h seira sou kai tote zhta na grapseis
            pipeArray[p]->wantW= 0;
            while (pipeArray[p]->turn);
            pipeArray[p]->wantW = 1;
        }
    }
    
    if (pipeArray[p]->w == pipeArray[p]->buffer + pipeArray[p]->pipeSize)
        pipeArray[p]->w = pipeArray[p]->buffer;
    
    *(pipeArray[p]->w) = c;
    (pipeArray[p]->w)++;
    pipeArray[p]->itmCount++;
    
    pipeArray[p]->turn = 1;
    pipeArray[p]->wantW = 0;
    
    return 1;
}