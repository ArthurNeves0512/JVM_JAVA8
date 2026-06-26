class Animal {
    public void falar() {
        System.out.println("Animal");
    }
}

class Cachorro extends Animal {
}

public class TesteHeranca {
    public static void main(String[] args) {
        Cachorro c = new Cachorro();
        c.falar();
    }
}