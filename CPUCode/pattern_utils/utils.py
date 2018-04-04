

def generateAccessPatterns_Atrace(stride_i, stride_j,Dimx, Dimy):
    '''
    Returns: a string in the Atrace format containing accesses relative to a Dimx X Dimy matrix strided on the x dimension
        by i and and on the y dimension
    '''
    res = ""
    for y in xrange(0,Dimy,stride_j):
        for x in xrange(0,Dimx,stride_i):
            res+="A["+str(y)+"]["+str(x)+"], "
    res+=";"
    return res


def generateAccessPatterns_Atrace_dense_linear_state(stride_i, stride_j,Dimx, Dimy):
    '''
    Returns: a string in the Atrace format containing accesses relative to a Dimx X Dimy matrix strided on the x dimension
        by i and and on the y dimension
    '''
    res = ""
    counter=0
    skip=False
    for y in xrange(0,Dimy):
        for x in xrange(0,Dimx):
            if skip and counter < stride_j:
                if counter < stride_j:
                        counter+=1
                else:
                        counter=0
                        skip=True
            else:
                if counter < stride_i:
                        counter+=1
                        res+="A["+str(y)+"]["+str(x)+"], "
                else:
                        counter=0
                        skip=False

            res+="A["+str(y)+"]["+str(x)+"], "
    res+=";"
    return res

def generateAccessPatterns_Atrace_dense(stride_i, stride_j,Dimx, Dimy):
    '''
    Returns: a string in the Atrace format containing accesses relative to a Dimx X Dimy matrix strided on the x dimension
        by i and and on the y dimension
    '''
    if stride_j >= stride_i:
        return ""

    res = ""
    for y in xrange(0,Dimy):
        for x in xrange(0,Dimx):
            if y%(stride_j) == 0 or  x%(stride_i) == 0:
                continue
            res+="A["+str(y)+"]["+str(x)+"], "
    res+=";"
    return res



def generateAccessPatterns_Atrace_columns(jump_every ,Dimx, Dimy):
    '''
    Returns: a string in the Atrace format containing accesses relative to a Dimx X Dimy matrix strided on the x dimension
        by i and and on the y dimension
    '''
    res = ""
    x=0
    while x < Dimx:
        for y in xrange(0,Dimy):
            res+="A["+str(y)+"]["+str(x)+"], "
        x+=jump_every+1
    res+=";"
    return res

def generateAccessPatterns_Atrace_rows(jump_every ,Dimx, Dimy):
    '''
    Returns: a string in the Atrace format containing accesses relative to a Dimx X Dimy matrix strided on the x dimension
        by i and and on the y dimension
    '''
    res = ""
    y=0
    while y < Dimy:
        for x in xrange(0,Dimx):
            res+="A["+str(y)+"]["+str(x)+"], "
        y+=jump_every+1
    res+=";"
    return res


def generateAccessPatterns_Atrace_rows_cols(jump_every_n_rows,jump_every_n_cols ,Dimx, Dimy):
    '''
    Returns: a string in the Atrace format containing accesses relative to a Dimx X Dimy matrix strided on the x dimension
        by i and and on the y dimension
    '''
    res = ""
    y=0
    while y < Dimy:
        x=0
        while x <Dimx:
            res+="A["+str(y)+"]["+str(x)+"], "
            x+=jump_every_n_cols+1
        y+=jump_every_n_rows+1
    res+=";"
    return res

def generateAccessPatterns_Atrace_linear_dense_new(read_n, skip_n ,Dimx, Dimy,starting_offset=0):
    '''
    Returns: a string in the Atrace format containing accesses relative to a Dimx X Dimy matrix strided on the x dimension
        by i and and on the y dimension
    '''
    res = ""
    i = starting_offset
    while i < Dimx*Dimy:
        for read_i in xrange(0, read_n):
            i += read_i
            res += "A[" + str(i / Dimx) + "][" + str(i % Dimx) + "], "
        i += skip_n +1
    res += ";"
    return res

def generateAccessPatterns_Atrace_linear_dense_new_correct(read_n, skip_n ,Dimx, Dimy,starting_offset=0):
    '''
    Returns: a string in the Atrace format containing accesses relative to a Dimx X Dimy matrix strided on the x dimension
        by i and and on the y dimension
    '''
    res = ""
    i = starting_offset
    while i < Dimx*Dimy:
        for read_i in xrange(0, read_n):
            res += "A[" + str(i / Dimx) + "][" + str(i % Dimx) + "], "
            i += 1
            if i >= Dimx*Dimy:
                break
        i += skip_n
    res += ";"
    return res


def generateAccessPatternsSOR(Dimx,Dimy,offset=False):
    res = ""
    if offset:
        for i in range(Dimy):
            for j in range(Dimx):
                if j % 2 != i % 2:
                    res += "A[" + str(i) + "][" + str(j) + "], "
    else:
        for i in range(Dimy):
            for j in range(Dimx):
                if j%2 == i%2:
                    res += "A[" + str(i ) + "][" + str(j) + "], "

    res += ";"
    return res


def powerset(seq):
    """
    Returns all the subsets of this set. This is a generator.
    """
    if len(seq) <= 1:
        yield seq
        yield []
    else:
        for item in powerset(seq[1:]):
            yield [seq[0]]+item
            yield item


def factors(n):
    """
    Returns all the factors of n
    """
    myList = []
    for i in xrange(1, int(n ** 0.5 + 1)):
        if n % i == 0:
            if (i != n/i):
                myList.append(i)
                myList.append(n / i)
            else:
                myList.append(i)
    return myList