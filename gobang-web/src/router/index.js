import { createRouter, createWebHistory } from 'vue-router'
import LoginPage from '../pages/LoginPage.vue'
import RegisterPage from '../pages/RegisterPage.vue'
import LobbyPage from '../pages/LobbyPage.vue'
import GameRoom from '../pages/GameRoom.vue'

const routes = [
  { path: '/', redirect: '/login' },
  { path: '/login', component: LoginPage },
  { path: '/register', component: RegisterPage },
  { path: '/lobby', component: LobbyPage },
  { path: '/room/:id', component: GameRoom },
]

const router = createRouter({
  history: createWebHistory(),
  routes,
})

export default router
