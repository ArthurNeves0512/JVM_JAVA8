class Animal {
    public void falar() {
        System.out.println("Animal");
    }
}

class Cachorro extends Animal {
    @Override
    public void falar() {
        System.out.println("AuAu");
    }
}

public class TestePolimorfismo {
    public static void main(String[] args) {
        Animal a = new Cachorro();
        a.falar();
    }
}