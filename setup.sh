cd ./peter/edax-reversi/data/
cp edax_book.dat mybook.dat
zip -9 peter_book.dat mybook.dat
rm mybook.dat

cd -
rm -f peter.zip
zip -9 peter.zip -r \
    ./peter/edax-reversi/bin \
    ./peter/edax-reversi/include \
    ./peter/edax-reversi/data/eval.dat \
    ./peter/edax-reversi/data/peter_book.dat \
    ./peter/edax-reversi/src \
    ./peter/__init__.py ./peter/player.py
rm ./peter/edax-reversi/data/peter_book.dat
