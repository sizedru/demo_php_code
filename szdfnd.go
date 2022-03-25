package main

import (
	"bufio"
	"encoding/json"
	"fmt"
	"log"
	"os"
	"regexp"
	"strings"

	_ "github.com/go-sql-driver/mysql"
)

type record struct {
	Id         int    `json:"id"`
	Surname    string `json:"surname"`
	Name       string `json:"name"`
	SecondName string `json:"secondname"`
	OrderNum   string `json:"order_num"`
	SudPrik    string `json:"sud_prik"`
}

var recsAll [500000]record

var recs []record

var recsCnt = 0

func getDb() {
	var rec record

	rd.InitDB("", "", "", "")
	defer rd.CloseDB()

	sqlStr := `
		select .................

	`

	rows, err := rd.QueryDB(sqlStr)
	rd.IsFail(err, "Запрос не удался")

	i := 0
	for rows.Next() {
		rows.Scan(&rec.Id, &rec.Surname, &rec.Name, &rec.SecondName, &rec.OrderNum, &rec.SudPrik)
		recsAll[i].Id = rec.Id
		recsAll[i].Surname = strings.ReplaceAll(strings.Trim(strings.ToLower(rec.Surname), " "), "ё", "е")
		recsAll[i].Name = strings.ReplaceAll(strings.Trim(strings.ToLower(rec.Name), " "), "ё", "е")
		recsAll[i].SecondName = strings.ReplaceAll(strings.Trim(strings.ToLower(rec.SecondName), " "), "ё", "е")
		recsAll[i].OrderNum = strings.Trim(strings.ToLower(rec.OrderNum), " ")
		recsAll[i].SudPrik = strings.Trim(strings.ToLower(rec.SudPrik), " ")
		i++
	}
	rows.Close()

	recsCnt = i

	recs = recsAll[0:recsCnt]
}

func replace(s string) string {

	s = strings.ToLower(s)

	s = " " + s + " "

	var re = regexp.MustCompile(`[0-9]|-|\/|\.|\(|\)|;|:|№|,`)
	s = re.ReplaceAllString(s, " ")

	s = strings.ReplaceAll(s, "взыскателю", " ")

	ms := [...]string{"перечисление", "средств", "погашения", "взыскателю", "судебный", "приказ", "инн", "кпп", "от", "лс", "в", "по", "счет", "долг", "долга", "г",
		"ип", "россия", "задолженности", "сумма", "без", "ндс", "ул", "кв", "д", "сп", "ип", "рн", "с", "л", "п",
	}

	for _, m := range ms {
		s = strings.ReplaceAll(s, " "+m+" ", " ")
	}

	so := strings.ReplaceAll(s, "ё", "е")
	so = strings.ReplaceAll(so, "  ", " ")
	for so != s {
		s = so
		so = strings.ReplaceAll(s, "  ", " ")
	}

	s = strings.Trim(s, " ")
	return s
}

func getName(s string) []record {
	a := strings.Split(s, " ")
	var ret [100]record
	mr := 0
	for i := range recs {
		m := 0
		for _, v := range a {
			if m == 0 {
				if v == recs[i].Surname {
					m = 1
				}
			} else if m == 1 {
				if v == recs[i].Name {
					m = 2
				} else {
					m = 0
					//break
				}
			} else if m == 2 {
				if v == recs[i].SecondName {
					m = 3
					break
				} else {
					m = 0
					//break
				}
			}
		}
		if m == 3 {
			ret[mr] = recs[i]
			mr++
		}
	}
	return ret[0:mr]
}

type ReturnData struct {
	StringId int
	Original string
	Retval   []record
}

func main() {

	var retData []ReturnData

	f := "read.txt"
	if len(os.Args) > 1 {
		f = os.Args[1]
	}

	file, err := os.Open(f)
	if err != nil {
		log.Fatal(err)
	}
	defer file.Close()

	getDb()

	scanner := bufio.NewScanner(file)
	i := 0
	for scanner.Scan() {
		o := scanner.Text()
		s := replace(o)
		n := getName(s)

		retData = append(retData, ReturnData{i, o, n})

		i++
	}

	if err := scanner.Err(); err != nil {
		log.Fatal(err)
	}

	myVar, _ := json.Marshal(retData)
	fmt.Println(string(myVar))
}
