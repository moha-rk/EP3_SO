#!/bin/bash

# script
rm fs.txt
echo Sistema de arquivos vazio
echo ---------------------- FS vazio \(1 e 4\) ----------------------
input1='./arquivos_teste/Teste1.txt'
input2='./arquivos_teste/Teste456.txt'
cria='./arquivos_teste/mount.txt'
for i in `seq 1 30`
do
    ./ep3 < $cria
    echo "$i";
    start=`date +%s%N`
    ./ep3 < $input1
    end=`date +%s%N`
    echo -n `expr $end - $start`", " >> './results/r11.txt'
    start=`date +%s%N`
    ./ep3 < $input2
    end=`date +%s%N`
    echo -n `expr $end - $start`", " >> './results/r14.txt'
    rm fs.txt
done


echo ---------------------- FS vazio \(2 e 5\) ----------------------
input1='./arquivos_teste/Teste2.txt'
input2='./arquivos_teste/Teste456.txt'
for i in `seq 1 30`
do
    ./ep3 < $cria
    echo "$i";
    start=`date +%s%N`
    ./ep3 < $input1
    end=`date +%s%N`
    echo -n `expr $end - $start`", " >> './results/r12.txt'
    start=`date +%s%N`
    ./ep3 < $input2
    end=`date +%s%N`
    echo -n `expr $end - $start`", " >> './results/r15.txt'
    rm fs.txt
done

echo ---------------------- FS vazio \(3 e 6\) ----------------------
input1='./arquivos_teste/Teste3.txt'
input2='./arquivos_teste/Teste456.txt'
for i in `seq 1 30`
do
    ./ep3 < $cria
    echo "$i";
    start=`date +%s%N`
    ./ep3 < $input1
    end=`date +%s%N`
    echo -n `expr $end - $start`", " >> './results/r13.txt'
    start=`date +%s%N`
    ./ep3 < $input2
    end=`date +%s%N`
    echo -n `expr $end - $start`", " >> './results/r16.txt'
    rm fs.txt
done

echo ---------------------- FS vazio \(7\) ----------------------
input1='./arquivos_teste/Teste78.txt'
cria30dirvazio='./arquivos_teste/PreparaTeste7.txt'
for i in `seq 1 30`
do
    ./ep3 < $cria30dirvazio
    echo "$i";
    start=`date +%s%N`
    ./ep3 < $input1
    end=`date +%s%N`
    echo -n `expr $end - $start`", " >> './results/r17.txt'
    rm fs.txt
done

echo ---------------------- FS vazio \(8\) ----------------------
input1='./arquivos_teste/Teste78.txt'
cria30dircheio='./arquivos_teste/PreparaTeste8.txt'
for i in `seq 1 30`
do
    ./ep3 < $cria30dircheio
    echo "$i";
    start=`date +%s%N`
    ./ep3 < $input1
    end=`date +%s%N`
    echo -n `expr $end - $start`", " >> './results/r18.txt'
    rm fs.txt
done

echo ============================================================
echo Sistema de arquivos com 10MB ocupados
echo ---------------------- FS 10MB \(1 e 4\) ----------------------
input1='./arquivos_teste/Teste1.txt'
input2='./arquivos_teste/Teste456.txt'
cria='./arquivos_teste/mount.txt'
preenche10='./arquivos_teste/preenche10.txt'
for i in `seq 1 30`
do
    ./ep3 < $cria
    ./ep3 < $preenche10
    echo "$i";
    start=`date +%s%N`
    ./ep3 < $input1
    end=`date +%s%N`
    echo -n `expr $end - $start`", " >> './results/r21.txt'
    start=`date +%s%N`
    ./ep3 < $input2
    end=`date +%s%N`
    echo -n `expr $end - $start`", " >> './results/r24.txt'
    rm fs.txt
done

echo ---------------------- FS 10MB \(2 e 5\) ----------------------
input1='./arquivos_teste/Teste2.txt'
input2='./arquivos_teste/Teste456.txt'
for i in `seq 1 30`
do
    ./ep3 < $cria
    ./ep3 < $preenche10
    echo "$i";
    start=`date +%s%N`
    ./ep3 < $input1
    end=`date +%s%N`
    echo -n `expr $end - $start`", " >> './results/r22.txt'
    start=`date +%s%N`
    ./ep3 < $input2
    end=`date +%s%N`
    echo -n `expr $end - $start`", " >> './results/r25.txt'
    rm fs.txt
done

echo ---------------------- FS 10MB \(3 e 6\) ----------------------
input1='./arquivos_teste/Teste3.txt'
input2='./arquivos_teste/Teste456.txt'
for i in `seq 1 30`
do
    ./ep3 < $cria
    ./ep3 < $preenche10
    echo "$i";
    start=`date +%s%N`
    ./ep3 < $input1
    end=`date +%s%N`
    echo -n `expr $end - $start`", " >> './results/r23.txt'
    start=`date +%s%N`
    ./ep3 < $input2
    end=`date +%s%N`
    echo -n `expr $end - $start`", " >> './results/r26.txt'
    rm fs.txt
done

echo ---------------------- FS 10MB \(7\) ----------------------
input1='./arquivos_teste/Teste78.txt'
cria30dirvazio='./arquivos_teste/PreparaTeste7.txt'
for i in `seq 1 30`
do
    ./ep3 < $preenche10
    ./ep3 < $cria30dirvazio
    echo "$i";
    start=`date +%s%N`
    ./ep3 < $input1
    end=`date +%s%N`
    echo -n `expr $end - $start`", " >> './results/r27.txt'
    rm fs.txt
done

echo ---------------------- FS 10MB \(8\) ----------------------
input1='./arquivos_teste/Teste78.txt'
cria30dircheio='./arquivos_teste/PreparaTeste8.txt'
for i in `seq 1 30`
do
    ./ep3 < $preenche10
    ./ep3 < $cria30dircheio
    echo "$i";
    start=`date +%s%N`
    ./ep3 < $input1
    end=`date +%s%N`
    echo -n `expr $end - $start`", " >> './results/r28.txt'
    rm fs.txt
done

echo ============================================================
echo Sistema de arquivos com 50MB ocupados
echo ---------------------- FS 50MB \(1 e 4\) ----------------------
input1='./arquivos_teste/Teste1.txt'
input2='./arquivos_teste/Teste456.txt'
cria='./arquivos_teste/mount.txt'
preenche50='./arquivos_teste/preenche50.txt'
for i in `seq 1 30`
do
    ./ep3 < $cria
    ./ep3 < $preenche50
    echo "$i";
    start=`date +%s%N`
    ./ep3 < $input1
    end=`date +%s%N`
    echo -n `expr $end - $start`", " >> './results/r31.txt'
    start=`date +%s%N`
    ./ep3 < $input2
    end=`date +%s%N`
    echo -n `expr $end - $start`", " >> './results/r34.txt'
    rm fs.txt
done

echo ---------------------- FS 50MB \(2 e 5\) ----------------------
input1='./arquivos_teste/Teste2.txt'
input2='./arquivos_teste/Teste456.txt'
for i in `seq 1 30`
do
    ./ep3 < $cria
    ./ep3 < $preenche50
    echo "$i";
    start=`date +%s%N`
    ./ep3 < $input1
    end=`date +%s%N`
    echo -n `expr $end - $start`", " >> './results/r32.txt'
    start=`date +%s%N`
    ./ep3 < $input2
    end=`date +%s%N`
    echo -n `expr $end - $start`", " >> './results/r35.txt'
    rm fs.txt
done

echo ---------------------- FS 50MB \(3 e 6\) ----------------------
input1='./arquivos_teste/Teste3.txt'
input2='./arquivos_teste/Teste456.txt'
for i in `seq 1 30`
do
    ./ep3 < $cria
    ./ep3 < $preenche50
    echo "$i";
    start=`date +%s%N`
    ./ep3 < $input1
    end=`date +%s%N`
    echo -n `expr $end - $start`", " >> './results/r33.txt'
    start=`date +%s%N`
    ./ep3 < $input2
    end=`date +%s%N`
    echo -n `expr $end - $start`", " >> './results/r36.txt'
    rm fs.txt
done

echo ---------------------- FS 50MB \(7\) ----------------------
input1='./arquivos_teste/Teste78.txt'
cria30dirvazio='./arquivos_teste/PreparaTeste7.txt'
for i in `seq 1 30`
do
    ./ep3 < $preenche50
    ./ep3 < $cria30dirvazio
    echo "$i";
    start=`date +%s%N`
    ./ep3 < $input1
    end=`date +%s%N`
    echo -n `expr $end - $start`", " >> './results/r37.txt'
    rm fs.txt
done

echo ---------------------- FS 50MB \(8\) ----------------------
input1='./arquivos_teste/Teste78.txt'
cria30dircheio='./arquivos_teste/PreparaTeste8.txt'
for i in `seq 1 30`
do
    ./ep3 < $preenche50
    ./ep3 < $cria30dircheio
    echo "$i";
    start=`date +%s%N`
    ./ep3 < $input1
    end=`date +%s%N`
    echo -n `expr $end - $start`", " >> './results/r38.txt'
    rm fs.txt
done
