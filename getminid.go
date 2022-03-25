package getminid

import (
	"fmt"
	"math/rand"
	"time"
)

type myInt int64

var demoDebug = true

var countForTest myInt = 100000

/*
	Задача: исходя из условий задачи строим однонаправленный список, содержащий элементы с уникальным индексом ID.
	Элементы не сортированы и при необходимости могут меняться операцией XOR. Требуется за минимальное число шагов
	найти минимальный незанятый индекс. Важно что элементы уникальны!
	Алгоритм: В каждый момент времени нам известна длина списка. Длинна списка символизирует максимально возможный
	незанятый индекс. Нумерация начинается с нуля. Например длина (количество) элементов  1000, значит  индексы  в
	списке могут быть от 0 до 999, и если все заняты, то есть номер ни одного элемента не превышает 999 тогда 1000
	это и есть искомое значение.
	Пример: список 0,1,6,5,4,7,8,12,9,20 - элементов 10. Минимальный 2.
	Как это найти: Делим длину пополам. Получаем 5. Считаем элементы которые меньше 5 и которые больше 5 но меньше
	10.	Если в первом случае количество элементов посчитанное меньше количества которое должно быть, то есть 5  то
	значит там не хватает элемента и искомый элемент меньше 5. Если значение = 5 то значит все элементы на месте и
	надо искать во втором списке. Если и там все на месте то значит еще выше.
	Поделили список и  в  следующий раз делаем  все  то же самое, но уже со списком который вдвое короче, и нам не
	приходится перебирать весь длинный список повторно.
	В примере элементов меньших 5 получилось 3 значит каких то не хватает. Мы берем нижний список  и  находим  что
	это число 2. Причем маленькие диапазоны например 20 элементов и меньше можно уже обработать простым перебором,
	что и сделано в предложенной программе.
	Алгоритм неплохо справляется с большими массивами данных. Протестирован на списках до 500 000 000 элементов  с
	средним результатом менее 2 секунд на поиск.
*/

// Элемент однонаправленного списка
type element struct {
	id   myInt
	next *element
}

// Однонаправленный список и количество элементов в нем
type spisok struct {
	elements *element
	count    myInt
}

var spisOk spisok

// Поиск минимального диапазона с искомым значением
func (s *element) getRange(mi, mx myInt) (min, max myInt, begin, end *element) {
	var (
		limits             [3]myInt
		countLo, countHi   myInt
		spisLo, spisLoLast *element
		spisHi, spisHiLast *element
		spisOh, spisOhLast *element
	)

	begin = s

	// Если элементов меньше 20 то используем простой перебор
	if mx-mi < 20 {
		min, max = mi, mx
		if s != nil {
			for true {
				if s.next == nil {
					break
				}
				s = s.next
			}
		}
		end = s
		return
	}

	// Вычисляем диапазоны значений для разделения списка
	limits[0] = mi
	limits[1] = limits[0] + (mx-mi)/2
	limits[2] = mx

	for true {
		if s.id >= limits[0] && s.id < limits[1] { // Список со значениями в нижнем диапазоне
			countLo++
			if spisLo == nil {
				spisLo = s
				spisLoLast = s
			} else {
				spisLoLast.next = s
				spisLoLast = spisLoLast.next
			}
		} else if s.id >= limits[1] && s.id < limits[2] { // Список со значениями в верхнем диапазоне
			countHi++
			if spisHi == nil {
				spisHi = s
				spisHiLast = s
			} else {
				spisHiLast.next = s
				spisHiLast = spisHiLast.next
			}
		} else { // Список элементов вне диапазонов. Элементы с номерами больше длины списка
			if spisOh == nil {
				spisOh = s
				spisOhLast = s
			} else {
				spisOhLast.next = s
				spisOhLast = spisOhLast.next
			}
		}
		if s.next == nil {
			break
		}
		s = s.next
	}

	if spisLoLast != nil {
		spisLoLast.next = nil
	}
	if spisHiLast != nil {
		spisHiLast.next = nil
	}
	if spisOhLast != nil {
		spisOhLast.next = nil
	}

	if countLo < limits[1]-limits[0] { // Если в нижнем списке не хватает элементов значит там искомый
		if countLo > 0 {
			min, max, spisLo, spisLoLast = spisLo.getRange(limits[0], limits[1]-1) // Ищем рекурсивно
		} else { // Если список пуст то первый элемент - искомый
			min, max = -1, limits[0]
		}
	} else if countHi < limits[2]-limits[1] { // Если в верхнем списке не хватает элементов значит там искомый
		if countHi > 0 {
			min, max, spisHi, spisHiLast = spisHi.getRange(limits[1], limits[2]-1) // Ищем рекурсивно
		} else { // Если список пуст то первый элемент - искомый
			min, max = -1, limits[1]
		}
	} else { // Если оба списка полны, значит искомый элемент последний
		min, max = -1, limits[2]
	}

	// Соединяем списки чтоб не "разорвался" исходный список.
	// Возвращаем ссылки на начало и конец списка и найденый диапазон для финального перебора

	if spisLo != nil {
		begin = spisLo
	} else if spisHi != nil {
		begin = spisHi
	} else {
		begin = spisOh
	}

	if spisLo != nil {
		if spisHi != nil {
			spisLoLast.next = spisHi
			end = spisHiLast
		} else if spisOh != nil {
			spisLoLast.next = spisOh
			end = spisOhLast
		} else {
			end = spisLoLast
		}
	}

	if spisHi != nil {
		if spisOh != nil {
			spisHiLast.next = spisOh
			end = spisOhLast
		} else {
			end = spisHiLast
		}
	}

	if spisOh != nil {
		end = spisOhLast
	}

	end.next = nil

	return
}

// Поиск следующего ID удовлетворяющего условию минимального значения
func (sps *spisok) getNext() myInt {
	var i, j, min, max myInt
	var start time.Time

	if demoDebug {
		start = time.Now()
	}

	min, max, sps.elements, _ = sps.elements.getRange(0, sps.count)

	// Если вернулся диапазон, а не конкретный элемент, то перебираем в пределах минимального диапазона.
	if min > -1 {
		for j = min; j <= max; j++ {
			f := false
			e := sps.elements
			for i = 0; i < sps.count; i++ {
				if e.id == j {
					f = true
					break
				}
				e = e.next
				if e == nil {
					break
				}
			}
			if !f {
				max = j
				break
			}
		}
	}

	if demoDebug {
		elapsed := time.Since(start)
		fmt.Printf("Время поиска %s\n", elapsed)
	}

	return max
}

// Добавление элемента в список
func (sps *spisok) addID() (id myInt) {
	if sps.count == 0 {
		id = 0
	} else {
		id = sps.getNext()
	}
	e := &element{}
	e.next = sps.elements
	e.id = id
	sps.elements = e
	sps.count++
	return
}

func (s *element) printSpis(n string) {
	fmt.Print(n, " ")
	if s != nil {
		ss := s
		i := 0
		for true {
			i++
			if i >= 200 {
				break
			}
			fmt.Print(ss.id, ":")
			if ss.next == nil {
				break
			}
			ss = ss.next
		}
	} else {
		fmt.Print("nil")
	}
	fmt.Println()
}

func (sps *spisok) printSpis(n string) {
	sps.elements.printSpis(n)
}

func (s *element) crypt(pwd myInt) {
	for true {
		s.id = s.id ^ pwd
		if s.next == nil {
			break
		}
		s = s.next
	}
}

func (sps *spisok) crypt(pwd myInt) {
	sps.elements.crypt(pwd)
}

func Run() {
	var i myInt
	var s *element
	spisOk.addID()

	demoDebug = true

	// Для тестов и отладки заполняем произвольными данными длинный список
	if demoDebug {
		s = spisOk.elements
		for i = 1; i < countForTest; i++ {
			s.next = &element{}
			s.next.id = i
			s = s.next
		}
		spisOk.count = countForTest

		spisOk.printSpis("SPS:")

		s = spisOk.elements
		for true {
			if s.id > 30 {
				if s.id != 97 {
					s.id += 50
				} else {
					s.id += countForTest
				}
			}
			s = s.next
			if s == nil {
				break
			}
		}
		spisOk.printSpis("SPS+50:")

		for i := 0; i <= 120; i++ {
			fmt.Println("ID: ", spisOk.addID())
			spisOk.crypt(myInt(rand.Intn(5)))
		}
	}

	// spisOk.addID() - добавляет элемент и возвращает найденный ID
	// spisOk.crypt(MyInt) - шифрует список
	// spisOk.printSpis(string) - печатает первые 200 элементов списка с комментарием string

}
