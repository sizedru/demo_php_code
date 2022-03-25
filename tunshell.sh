#!/bin/bash

# *************   Const    *************
TmpDir="/home/sized/.tmp"
TmpDir="/home/sshtun/.tmp"

SysSSHD="0.0.0.0:443"

LocalHost="127.0.0.1:"
All5900=$LocalHost"59"

FirstPort=5900

SSHcliUser="sized"
SSHadmUser="sized"

SSHcliUser="sshtun"
SSHadmUser="sshtun"


# *************    Var    *************
Ports=()				# Массив портов
AclPorts=()				# Массив разрешенных портов ??? Реализуем потом для ограничения... чтоб не заDOSили
Params=()				# Массив параметров

# Системные переменные
cl_ip="127.0.0.1"		# IP адрес клиента
cl_port="0"				# Порт подключения системный... не равен 443... назначается автоматически
cl_fw_port="0"			# Порт перенаправления... 5900... и т.д.
cl_sshd_pid="0"			# PID процесса sshd с которым клиент соединился - главный PID
cl_tunn_pid="0"			# PID процесса sshd организующего проброс портов
cl_shel_pid="0"			# PID текущего запущенного shella

TmpFile=""				# Файл идентификации подключения

# *************  Declare  *************

# Функция: pause
# Описание: Ждет нажатия любой клавиши.
# Использование: pause 'Press any key to continue...'
function pause(){
   echo "$*" # Почему-то read не выводит строку через ssh
   read -p "$*"
}

# Функция: parray
# Описание: Превращает параметры переданные скрипту в массив параметров Params[]
# Использование: parray
function parray(){
	let "i=0"
	for p in $*; do
		Params[$i]=$p
		let "i=i+1"
	done
}

# Функция getsys
# Описание: Заполняет системные переменные
# Использование: getsys
function getsys(){
	if [ "X${SSH_CLIENT}" != "X" ]; then
		let "i=1"
		for p in $SSH_CLIENT; do
			if [ $i == 1 ]; then
				cl_ip=$p
			elif [ $i == 2 ]; then
				cl_port=$p
			fi
			let "i=i+1"
		done
		cl_sshd_pid=`netstat -4anp | grep $cl_port | awk '{print $7;}' | sed "s/\/sshd://g"`
		#cl_tunn_pid=$PPID # Так не очень эффективно... при отладке вызывался скрипт из BASH и возвращял его PID а не PID sshd
		cl_tunn_pid=`ps -u $SSHcliUser -l | grep $cl_sshd_pid | awk '{print $4;}'`
		cl_shel_pid=$$

		if [ "X${cl_tunn_pid}" != "X" ]; then
			cl_fw_port=`netstat -4anp | grep $cl_tunn_pid | awk '{print $4;}' | sed "s/$LocalHost//g"`
		else
			cl_fw_port=""
		fi
	fi
}

# Функция FindFreePort
# Описание: Находит свободный порт для реализации форвардинга
#           Требует предварительного заполнения массива Ports, хотя и без этого сработает :)
# Использование: `FindFreePort`
function FindFreePort(){
	# Алгоритм поиска немного избыточен, однако мне кажется он необходим
	# для того, чтоб если существует временный файл с указанным портом
	# то порт бы не использовался по новой даже если нет процесса использующего порт.
	# В общем такая вот загогулина получилась...
	port=$FirstPort
	for item in "${Ports[@]}"; do
		# Если есть во временных файлах то пропускаем
		if [ "$port" == "$item" ]; then
			let "port=port+1"
		else
			# Если нету то проверяем а не системный ли это порт, не занят ли он другим процессом
			ns=`netstat -4an | grep $port`
			if [ "X${ns}" != "X" ]; then
				let "port=port+1"
			fi
		fi
	done
	
	# На всякий случай, вдруг это первое соединение и еще нет временных файлов
	ns=`netstat -4an | grep $port`
	while [ "X${ns}" != "X" ]
	do
		let "port=port+1"
		ns=`netstat -4an | grep $port`
	done

	echo $port
	# Вместо return мы просто печатаем результат, т.к. return ограничено значением 255 (один байт)
	# а функцию будем использовать в виде `FindFreePort`
}


# *************   Begin   *************

parray $@

## Вывод на экран массива параметров
#for item in "${Params[@]}"; do
#    echo $item
#done

# Для основного параметра используем первый элемент массива параметров
# т.к. при работе по SSH нулевой параметр будет "-c"
par=${Params[1]}

getsys

TmpFile=$TmpDir/$cl_ip.$cl_port

#echo $TmpFile


ls=`ls $TmpDir/`
if [ "X${ls}" != "X" ] ; then # Это чтобы если каталог пустой, то не ругалось бы в следующих контрукциях
	# Найдем все занятые порты
	Ports=( `(for p in $TmpDir/*; do sed '3!d' $p; done) | sort` )

	# Если ссессия закрывается некорректно, то остается временный файл, его надо убить если нет процесса
	# ??? надо подумать может лучше через netstat... типа ip и port... они есть кстати в имени файла
	for p in `ls $TmpDir/*`; do
		# Если процесса tunnel нету тогда
		se=`sed '5!d' $p;`
		ps=`ps -u $SSHcliUser | awk '{print "X"$1"X";}' | grep "X"$se"X" | sed "s/X//g"`
		if [ "X${ps}" == "X" ] ; then
			# Удаляем файл
			rm $p 2>/dev/null
			#echo "werwerwewerwre"
		fi
	done
fi

if [ "$par" == "get" ]; then
	echo `FindFreePort` # Ну для понта ;)
#elif [ "$par" == "bash" ]; then
#    /bin/bash
elif [ "$par" == "windowsN" ]; then
	port=`FindFreePort`
	echo "plink.exe -pw 11 -R $port:localhost:5900 -l sshtun -i sshtunpriv.ppk -load sized connect %COMPUTERNAME%"
#	echo "plink.exe"
#	echo "tskill WinVNC"

elif [ "$par" == "windows" ]; then
	port=`FindFreePort`
	echo "plink.exe -pw 11 -R $port:localhost:5900 -l sshtun -i sshtunpriv.ppk -load sized connect %COMPUTERNAME%"
#	echo "plink.exe"
#	echo "tskill WinVNC"
#	echo "ping sized.lipetsk.ru"
#	echo "WinVNC.exe"

#	echo "tskill WinVNC"
#	echo "tskill plink"
#	echo "tskill"
elif [ "$par" == "rdp" ]; then
#	echo "plink.exe -pw 11 -R 192.168.1.33:3389:localhost:3389 -l sshtun -i sshtunpriv.ppk -load sized admin"
	echo "plink.exe -pw 11 -L 3128:localhost:3128 -l sshtun -i sshtunpriv.ppk -load sized admin"
#	echo "tskill cmd"
#	echo "tskill WinVNC"
#	echo "ping sized.lipetsk.ru"
#	echo "WinVNC.exe"
#	echo "tskill plink"

elif [ "$par" == "show" ]; then
	ls=`ls $TmpDir/`
	if [ "X${ls}" != "X" ] ; then # Это чтобы если каталог пустой, то не ругалось бы в следующих контрукциях
		greppattern="";
		for i in `netstat -4an | grep "ESTABLISHED" | grep "$All5900" | awk '{print $4;}' | grep "$All5900"`; do
			if [ "X${greppattern}" != "X" ] ; then
				greppattern=${greppattern}"|"
			fi
			greppattern=${greppattern}${i}
		done

		Connections=()

		if [ "X${greppattern}" == "X" ] ; then
			Connections=( "${Connections[@]}" "LISTEN:" )

			ns=`netstat -4an | grep "LISTEN" | grep "$All5900" |  awk '{print $4;}' | sed "s/$LocalHost//g"`
			Connections=( "${Connections[@]}" $ns )
		else
			Connections=( "${Connections[@]}" "CONNECTED:" )
			ns=`netstat -4an | grep "ESTABLISHED" | grep -E $greppattern | awk '{print $4;}' | grep "$All5900" | sed "s/$LocalHost//g"`
			Connections=( "${Connections[@]}"  $ns )

			ns=`netstat -4an | grep "LISTEN" | grep "$All5900" | grep -v -E $greppattern | awk '{print $4;}' | sed "s/$LocalHost//g"`
			Connections=( "${Connections[@]}" "LISTEN:" )
			Connections=( "${Connections[@]}" $ns )
		fi

		PORTs=()
		FILEs=()
		for p in `ls $TmpDir/*`; do
			sePORT=`sed '3!d' $p;`
			PORTs=( "${PORTs[@]}" $sePORT )
			FILEs=( "${FILEs[@]}" $p )
		done
	
		for item in "${Connections[@]}"; do
			let "i=0"
			seIP=""
			seKEY=""
			sePORT=$item
			for p in "${PORTs[@]}"; do
				if [ "$p" == "$item" ]; then
					seIP=`sed '1!d' ${FILEs[$i]};`
					seKEY=`sed '7!d' ${FILEs[$i]};`
					break
				fi
				let "i=i+1"
			done
			echo "$seIP:$sePORT+$seKEY"
		done
	fi
elif [ "$par" == "connect" ]; then
	# Создадим файл идентификации соединения
	touch $TmpFile

	# Запишем в него идентификационную информацию в формате:
	#   IP клиента
	#   Порт клиента
	#   Порт перенаправления
	#   PID sshd
	#   PID forwarder
	#   PID shell
	#   Идентификатор, ключевое слово, имя комптютера, название организации переданное в дополнительных параметрах
	echo $cl_ip > $TmpFile
	echo $cl_port >> $TmpFile
	echo $cl_fw_port >> $TmpFile
	echo $cl_sshd_pid >> $TmpFile
	echo $cl_tunn_pid >> $TmpFile
	echo $cl_shel_pid >> $TmpFile
	echo ${Params[2]} >> $TmpFile

	# Если порт был занят уже кем-то... тогда программа не отработает... значит ошибка...
	if [ "X${cl_fw_port}" != "X" ] ; then
		pause 'Press any key to continue...'
	else
		echo "Error" # Надо будет доработать систему возврата ошибок...
					 # возможно статус возврата использовать, но надо чтоб этот статус как-то влиял
					 # на статус возврата возвращаемый процессом plink на клиентской стороне... вот...
	fi

	# Удалим файл идентификации соединения
	rm $TmpFile
elif [ "$par" == "admin" ]; then
	pause 'Press any key to continue...'
elif [ "X${par}" == "X" ]; then
	pause 'Press any key to continue...'
else
 	echo "Only for system application..."
fi

#pause 'Press any key to continue...'
