// Кусочек кода тестового вывода растянутого по ширине текста в pdf
// из git истории проекта по генерации pdf документов

bool wr = false;

void RWin::pdf(QString filename, QPixmap &pm) {
    QPdfWriter writer(filename);
    writer.setResolution(300);
    writer.setPageSize(QPageSize::PageSizeId::A4);
    writer.setPageMargins(QMargins(10, 1, 3, 3));

    QPainter p(&writer);

    p.drawPixmap(0, 0, pm);
    qInfo() << "pdf w: " << writer.width();
    qInfo() << "pdf h: " << writer.height();

    p.end();
}


void RWin::paint(QPaintEvent *event) {
    QPixmap pm(QSize(this->sx, this->sy));
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);
    p.fillRect(QRect(0, 0, pm.width(), pm.height()), Qt::white);
    p.setPen(Qt::black);

    QFont font1("Roboto", 26);
    p.setFont(font1);
    int beginPosY = this->doc.Rect.y() + QFontMetrics(p.font()).height();

    QStringList list = this->doc.Text.split(QRegularExpression("[\r\n\t ]+"), Qt::SplitBehaviorFlags::SkipEmptyParts);
    int j = 0;
    QStringList li;
    bool isError = false;
    bool beginTab = true;
    for (int i = 0; i < list.size(); ++i) {
        QString str = list.at(i) + " ";
        if (beginTab) {
            str = "                " + str;
            beginTab = false;
        }
        if (j + QFontMetrics(p.font()).size(Qt::TextSingleLine, str).width() <= this->doc.Rect.width() - 10) {
            li.append(str);
            j += QFontMetrics(p.font()).size(Qt::TextSingleLine, str).width();
        } else {
            if (beginPosY < this->doc.Rect.height()) {
                int beginPosX = this->doc.Rect.x() + 10;
                int jj = 0;
                int jp = 0;
                if (li.size() > 1) {
                    jj = (this->doc.Rect.width() - 10 - j - QFontMetrics(p.font()).size(Qt::TextSingleLine, " ").width()) / (li.size() - 1);
                    jp = (this->doc.Rect.width() - 10 - j - QFontMetrics(p.font()).size(Qt::TextSingleLine, " ").width()) % (li.size() - 1);
                }

                for (int k = 0; k < li.size(); k++) {
                    p.drawText(beginPosX, beginPosY, li.at(k));
                    beginPosX += QFontMetrics(p.font()).size(Qt::TextSingleLine, li.at(k)).width() + jj + jp;
                    jp = 0;
                }
                beginPosY += QFontMetrics(p.font()).ascent() + 5;
                li.clear();
                li.append(list.at(i) + " ");
                j = QFontMetrics(p.font()).size(Qt::TextSingleLine, list.at(i) + " ").width();
            } else {
                isError = true;
            }
        }
    }

    if (beginPosY < this->doc.Rect.height()) {
        int beginPosX = this->doc.Rect.x() + 10;
        for (int k = 0; k < li.size(); k++) {
            p.drawText(beginPosX, beginPosY, li.at(k));
            beginPosX += QFontMetrics(p.font()).size(Qt::TextSingleLine, li.at(k)).width();
        }
    } else {
        isError = true;
    }

    if (isError) {
        p.setPen(QPen(Qt::red, 3, Qt::SolidLine));
    } else {
        p.setPen(QPen(Qt::blue, 3, Qt::SolidLine));
    }
    p.drawRect(this->doc.Rect); 


    QPainter pp(this);

    pp.drawPixmap(0, 0, pm);

    if (!wr) {
        pm.save("NEW.jpg", "JPEG");
        pdf("NEW.pdf", pm);
        wr = true;
    }
}
