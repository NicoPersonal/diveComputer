// TableHelper.hpp
#ifndef TABLE_HELPER_HPP
#define TABLE_HELPER_HPP

#include "qtheaders.hpp"
#include <functional>

namespace DiveComputer {

class TableHelper {
public:
    // Configure basic table properties
    static void configureTable(QTableWidget* table, 
                              QAbstractItemView::SelectionBehavior behavior = QAbstractItemView::SelectRows,
                              QAbstractItemView::SelectionMode mode = QAbstractItemView::SingleSelection) {
        if (!table) return;
        
        table->setSelectionBehavior(behavior);
        table->setSelectionMode(mode);
        table->setAlternatingRowColors(true);
        table->verticalHeader()->setVisible(false);
        
        // Common edit triggers
        table->setEditTriggers(QAbstractItemView::DoubleClicked | 
                              QAbstractItemView::SelectedClicked | 
                              QAbstractItemView::EditKeyPressed);
    }
    
    // Set table headers and column properties
    static void setHeaders(QTableWidget* table, const QStringList& headers) {
        if (!table || headers.isEmpty()) return;
        
        table->setColumnCount(headers.size());
        table->setHorizontalHeaderLabels(headers);
    }
    
    // Safe table update with signal blocking
    template<typename T, typename Func>
    static void safeUpdate(QTableWidget* table, T* instance, Func cellChangedHandler, 
                          std::function<void()> updateFunc) {
        if (!table || !instance || !updateFunc) return;
        
        // Disconnect cell change signals temporarily
        QObject::disconnect(table, &QTableWidget::cellChanged, instance, cellChangedHandler);
        
        // Clear and reset the table
        table->clearContents();
        
        // Execute the update function
        updateFunc();
        
        // Reconnect signals
        QObject::connect(table, &QTableWidget::cellChanged, instance, cellChangedHandler);
    }
    
    // Create non-editable cell with center alignment
    static QTableWidgetItem* createReadOnlyCell(const QString& text) {
        QTableWidgetItem* item = new QTableWidgetItem(text);
        item->setTextAlignment(Qt::AlignCenter);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable); // Make non-editable
        return item;
    }
    
    // Create numeric cell with proper formatting
    static QTableWidgetItem* createNumericCell(double value, int precision = 1, bool editable = false) {
        QTableWidgetItem* item = new QTableWidgetItem(QString::number(value, 'f', precision));
        item->setTextAlignment(Qt::AlignCenter);
        
        if (!editable) {
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        }
        
        return item;
    }
    
    // Highlight cell based on condition
    static void highlightCell(QTableWidgetItem* item, bool condition, 
                            const QColor& color = QColor(255, 200, 200)) {
        if (!item) return;
        
        if (condition) {
            item->setBackground(QBrush(color));
        } else {
            item->setBackground(QBrush());
        }
    }
};

} // namespace DiveComputer

#endif // TABLE_HELPER_HPP