class Animal {
    int idade;
}

class Cachorro2 extends Animal {

    void teste() {
        idade = 7;
        System.out.println(idade);
    }

    public static void main(String[] args) {
        new Cachorro().teste();
    }
}