<?php

/*
 * This file is part of elliotwright.co
 *
 * (c) Elliot Wright <elliot@elliotwright.co>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

namespace SeerUK\Module\SecurityModule\Authentication\Authenticator;

use Aegis\Authentication\Authenticator\AuthenticatorInterface;
use Aegis\Exception\AuthenticationException;
use Aegis\Token\TokenInterface;
use Doctrine\ORM\EntityRepository;
use SeerUK\Module\SecurityModule\Authentication\Validator\UserValidator;
use SeerUK\Module\SecurityModule\Data\Entity\User;
use SeerUK\Module\SecurityModule\Token\UserToken;
use Trident\Component\Caching\Caching;

/**
 * User Authenticator
 *
 * @author Elliot Wright <elliot@elliotwright.co>
 */
class UserAuthenticator implements AuthenticatorInterface
{
    private $caching;
    private $repository;
    private $validator;

    /**
     * Constructor.
     *
     * @param EntityRepository $repository
     */
    public function __construct(Caching $caching, EntityRepository $repository, UserValidator $validator)
    {
        $this->caching = $caching;
        $this->repository = $repository;
        $this->validator = $validator;
    }

    /**
     * {@inheritDoc}
     */
    public function authenticate(TokenInterface $token)
    {
        if ( ! isset($this->repository)) {
            throw new \RuntimeException('No repository found to fetch user from.');
        }

        $username = $token->getCredentials()['username'];
        $key = "sm.authenticator.user.fetch.$username";

        if ( ! $user = $this->caching->get($key)) {
            $user = $this->repository->findOneByUsername($username);

            $this->caching->set($key, $user, 120);
        }

        if ( ! $user || ! $this->validator->validate($token, $user)) {
            throw new AuthenticationException($token, 'Bad credentials.');
        }

        $token->setUser($user);
        $token->setAuthenticated(count($user->getRoles() > 0));

        return $token;
    }

    /**
     * {@inheritDoc}
     */
    public function supports(TokenInterface $token)
    {
        return ($token instanceof UserToken);
    }
}
