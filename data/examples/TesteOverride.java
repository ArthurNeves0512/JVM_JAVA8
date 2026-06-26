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

public class TesteOverride {
    public static void main(String[] args) {
        Cachorro c = new Cachorro();
        c.falar();
    }
}