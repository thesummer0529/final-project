using UnityEngine;

public class Player : MonoBehaviour
{
    public int health = 100;
    public int attackPower = 10;
    
    void Update()
    {
        if (health <= 0)
        {
            Die();
        }
    }

    public void TakeDamage(int damage)
    {
        health -= damage;
    }

    public void Attack(Enemy enemy)
    {
        enemy.TakeDamage(attackPower);
    }

    private void Die()
    {
        Debug.Log("player die! restart");
        GameManager.instance.RestartGame();
    }
}
