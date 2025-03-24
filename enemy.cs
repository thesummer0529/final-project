using UnityEngine;

public class Enemy : MonoBehaviour
{
    public int health = 50;
    public int damage = 5;

    public void TakeDamage(int damage)
    {
        health -= damage;
        if (health <= 0)
        {
            Die();
        }
    }

    private void Die()
    {
        Debug.Log("敵人已被擊敗！");
        Destroy(gameObject);
    }
}
