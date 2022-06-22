from sys import argv

def n_queens_expression_output(n: int) -> str:
    if n < 2:
        raise Exception("illegal n")
    # variable
    P = [[f'P{i}{j}' for j in range(n)] for i in range(n)]
    row = ''
    for i in range(n):
        row_min = ''
        for j in range(n):
            row_min += (P[i][j] + '|')
        row_max = ''
        for l in range(n):
            for k in range(l):
                row_max += f'(~{P[i][k]}|~{P[i][l]})&'
        row += f'({row_min[:-1]})&({row_max[:-1]})&'
    col = ''
    for i in range(n):
        col_min = ''
        for j in range(n):
            col_min += (P[j][i] + '|')
        col_max = ''
        for l in range(n):
            for k in range(l):
                col_max += f'(~{P[k][i]}|~{P[l][i]})&'
        col += f'({col_min[:-1]})&({col_max[:-1]})&'
    diag_neg = ''
    for k in range(2*n-1):
        diag_neg_k = ''
        for i in range(max(0, k-n+1), min(k, n)):
            for j in range(i+1, min(n, k+1)):
                diag_neg_k += f'(~{P[i][k-i]}|~{P[j][k-j]})&'
        diag_neg += diag_neg_k
    diag_pos = ''
    for k in range(1-n, n):
        diag_pos_k = ''
        for i in range(max(0, k), min(n+k, n)):
            for j in range(i+1, min(n+k, n)):
                diag_pos_k += f'(~{P[i][i-k]}|~{P[j][j-k]})&'
        diag_pos += diag_pos_k
    return row + col + diag_neg + diag_pos[:-1]


if __name__ == "__main__":
    try:
        print(n_queens_expression_output(int(argv[1])))
    except:
        exit(-1)
