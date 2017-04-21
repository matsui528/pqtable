import numpy as np
import argparse

def read_score(path):
    """
    Given a path of score, reads IDs.
    :param path: A path of score file
    :return: a (Nq x k) numpy array. Nq is the number of query, k is top_k result/
    """
    lines = [line for line in open(path, 'rt')][2:]  # First two lines are header, then skip
    lines = [line.split(',')[0::2][:-1] for line in lines]  # For each line (each query), split id and scores, select ids, then pop the final \n
    lines = [[int(elem) for elem in line] for line in lines]  # Convert to int
    # line[q][k] q-th query, top_k-th id
    return np.array(lines)

def read_xvec(path, base_type, N=-1):
    """
    A utility function to read YAEL format files
    (xxx.ivec, xxx.fvec, and xxx.bvec)
    :param path: The path of xvec file
    :param base_type: The type of xvec; 'f', 'i' or 'b'
    :param N: The number of vectors to be read. If this is not specified, read all
    :return: a N x dim array
    """
    import struct  # for handling binary data
    import os

    assert(base_type == 'f' or base_type == 'i' or base_type == 'b')

    if base_type == "b":  # Convert 'b' to 'B' because unsinged char is 'B' for struct
        base_type = 'B'

    data_type, byte_size = {
        'f': (np.float32, 4),
        'i': (np.int32, 4),
        'B': (np.uint8, 1)
    }[base_type]

    size = os.path.getsize(path)

    with open(path, 'rb') as fin:
        bdata = fin.read(4)  # the first 4 byte is a number of dimension
        dim = struct.unpack('i', bdata)[0]
        N_all = int(size / (4 + dim * byte_size)) # All size

        if N == -1:
            N = N_all
        assert(N <= N_all)

        fin.seek(0) # Set cursor to the initial position
        vec = np.empty((N, dim), dtype=data_type)

        for n in range(N):
            bdata = fin.read(4) # the fist 4 byte is always dim, so skip it
            bdata = fin.read(byte_size * dim) # Read a vector
            vec[n, :] = np.array(struct.unpack(base_type * dim, bdata), dtype=data_type)
    return vec


class RecallEvaluator(object):
    def __init__(self, ranked_ids, groundtruths):
        assert(ranked_ids.ndim == 2)
        assert(groundtruths.ndim == 1)
        assert(ranked_ids.shape[0] == groundtruths.shape[0])

        self.ranked_ids = ranked_ids
        self.groundtruths = groundtruths


    def recall_at_r(self, R):
        k_query = self.ranked_ids.shape[0]
        k_ranked_ids_len = self.ranked_ids.shape[1]

        assert(R <= k_ranked_ids_len)
        recall_at_r = 0.0
        for q in range(k_query):
            for r in range(R):
                if self.ranked_ids[q][r] == self.groundtruths[q]:
                    recall_at_r += 1.0

        recall_at_r /= k_query
        return recall_at_r


parser = argparse.ArgumentParser()
parser.add_argument("score")
parser.add_argument("groundtruth")
args = parser.parse_args()



gt = read_xvec(args.groundtruth, 'i')[0:, 0]  # just use the top-1 groundtruth
score = read_score(args.score)
top_k = score.shape[1]

eval = RecallEvaluator(score, gt)

for R in [1, 2, 5 ,10, 20, 50, 100]:
    if R <= top_k:
        print("Recall@" + str(R) + ": {0:.3f}".format(eval.recall_at_r(R)))




