from PyQt6.QtWidgets import QApplication, QWidget, QFileDialog, QTableWidgetItem, QHeaderView, QMessageBox
from PyQt6.QtCore import Qt
from PyQt6.QtGui import QFont
from widget import Ui_Widget
from backend import Backend

import sys
import os

class App(QWidget, Ui_Widget):
    def __init__(self):
        super().__init__()
        self.setFixedSize(1000,750)
        self.__data: list[list[list[str]]] = []
        self.setupUi(self)
        self.setWindowTitle("Seat Allocator")
        self.csv_import.clicked.connect(self.__read_file)

        self.seat_viewer.horizontalHeader().setSectionResizeMode(QHeaderView.ResizeMode.Stretch)
        self.seat_viewer.verticalHeader().setSectionResizeMode(QHeaderView.ResizeMode.Stretch)

        self.tab_display.removeTab(0)
        self.tab_display.setTabText(0, "原始")
        self.tab_display.tabBarClicked.connect(lambda index: self.__display_data(index))
        del self.tab
        del self.tab_2

        self.begin_shuffle.clicked.connect(self.__begin_shuffling)

        self.delete_all_button.clicked.connect(self.__confirm_delete_all)

        self.export_button.clicked.connect(self.__export_data)


    #  --------------------------------------------------------------------------
    def __read_file(self) -> None:
        """
        Open a file dialog to select a CSV file and read its data.
        Updates the seat_viewer table with the data from the CSV file.
        :return: Nothing.
        """
        file_path, _ = QFileDialog.getOpenFileName(self, "Open CSV", "", "CSV Files (*.csv)")
        if not file_path:
            print("Failed to read CSV data.")
            return

        print(f"Selected file: {file_path}")
        data = Backend.read_csv(file_path)
        if data:
            print("CSV Data:")
            # 清空 __data
            self.__delete_all(True)
            self.__data.append(data)

            self.begin_shuffle.setEnabled(True)
            self.csv_import.setText(f"{os.path.basename(file_path)}")
            self.csv_import_state.setText("已導入")
            self.__display_data(0)
            for row in data:
                print(row)
        else:
            QMessageBox.warning(self, "錯誤", "讀取 CSV 檔案失敗。")


    #  --------------------------------------------------------------------------
    def __display_data(self, index) -> None:
        """
        Display the data at the given index in the seat_viewer table.
        :param index: the index of the data to display
        :return: Nothing. It only updates the UI.
        """
        if 0 <= index < len(self.__data) and self.__data:
            data = self.__data[index]
            row = len(data)
            col = len(data[0]) if row > 0 else 0
            self.seat_viewer.setRowCount(row)
            self.seat_viewer.setColumnCount(col)

            for i in range(row):
                for j in range(col):
                    item = QTableWidgetItem(data[i][j])
                    item.setTextAlignment(Qt.AlignmentFlag.AlignCenter)
                    item.setFont(QFont("Microsoft JhengHei", 16))
                    self.seat_viewer.setItem(i, j, item)

    #  --------------------------------------------------------------------------
    def __begin_shuffling(self) -> None:
        """
        Begin shuffling the seats based on the original data.
        This function uses the allocate_seats function from the backend to shuffle the data.
        It then updates the tab display and shows the shuffled data in the seat_viewer table.
        :return: Nothing. It only updates the UI.
        """
        assert self.__data, "Data is empty"
        target_data = self.__data[0]
        try:
            shuffled_data = Backend.allocate_seats(target_data)
            self.__data.append(shuffled_data)
            self.tab_display.addTab(QWidget(), f"第{len(self.__data)-1}次打亂")
            self.tab_display.setCurrentIndex(len(self.__data)-1)
            self.__display_data(len(self.__data)-1)
        except RuntimeError as e:
            QMessageBox.critical(self, "錯誤", str(e))

    #  --------------------------------------------------------------------------
    def __delete_all(self, delete_first: bool = False) -> None:
        """
        Clear all scramble records.
        :return: Nothing. It only updates the UI.
        """
        if not self.__data:
            return
        self.__display_data(0)
        self.__data = [] if delete_first else self.__data[:1]
        self.tab_display.setCurrentIndex(0)
        while self.tab_display.count() > 1:
            self.tab_display.removeTab(1)

    #  --------------------------------------------------------------------------
    def __confirm_delete_all(self) -> None:
        """
        Confirm deletion of all scramble records.
        :return: Nothing. It only updates the UI.
        """
        if not self.__data:
            return
        reply = QMessageBox.question(self, '確認刪除', "確定要刪除所有打亂紀錄嗎？",
                                     QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No, QMessageBox.StandardButton.No)
        if reply == QMessageBox.StandardButton.Yes:
            self.__delete_all()

    #  --------------------------------------------------------------------------
    def __export_data(self):
        """
        Export the current data to a CSV file.
        :return: Nothing. It only updates the UI.
        """
        if not self.__data or (current := self.tab_display.currentIndex()) == 0:
            return
        file_path, _ = QFileDialog.getSaveFileName(self, "Save CSV", "", "CSV Files (*.csv)")
        if not file_path:
            print("Failed to save CSV data.")
            return
        try:
            import pandas as pd
            df = pd.DataFrame(self.__data[current])
            df.to_csv(file_path, header=False, index=False)
            QMessageBox.information(self, "成功", f"已成功把第{current}次打亂記錄匯出至{file_path}")
            print(f"Data exported to {file_path}")
        except Exception as e:
            print(f"Error exporting CSV file: {e}")

if __name__ == '__main__':
    app = QApplication(sys.argv)
    window = App()
    window.show()
    sys.exit(app.exec())