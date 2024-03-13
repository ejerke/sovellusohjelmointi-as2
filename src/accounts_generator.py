import random
random.seed(1234)
with open("accounts.txt", "w") as file:
    for i in range(1, 20):
        s = ""
        s += str(i) + " " + str(random.randint(0, 2000)) + "\n"
        file.write( s )
