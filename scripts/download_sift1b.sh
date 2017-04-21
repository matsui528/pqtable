# Download and extract sift1b vectors on data/

# Takes time
wget ftp://ftp.irisa.fr/local/texmex/corpus/bigann_base.bvecs.gz -P data
wget ftp://ftp.irisa.fr/local/texmex/corpus/bigann_learn.bvecs.gz -P data
wget ftp://ftp.irisa.fr/local/texmex/corpus/bigann_query.bvecs.gz -P data
wget ftp://ftp.irisa.fr/local/texmex/corpus/bigann_gnd.tar.gz -P data

gunzip -c data/bigann_base.bvecs.gz > data
gunzip -c data/bigann_learn.bvecs.gz > data
gunzip -c data/bigann_query.bvecs.gz > data
tar -zxvf data/bigann_gnd.tar.gz -C data
