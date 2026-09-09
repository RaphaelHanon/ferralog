import sqlite3 
conexao = sqlite3.connect('ferralog.db')
cursor = conexao.cursor()

cursor.execute('''CREATE TABLE IF NOT EXISTS ferralog(id INTEGER PRIMARY KEY AUTOINCREMENT, 
nome TEXT,valor REAL, qtd INTEGER, qr INTEGER, garantia TEXT ) ''')



cursor.execute(""" INSERT INTO ferralog(nome,valor,qtd,qr,garantia) VALUES (?,?,?,?,?)""",(nome,valor,qtd,qr,garantia))
conexao.commit()
cursor.close()
conexao.close()
